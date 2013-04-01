#include "crt.h"
#include "helpers.h"

void crt_scrollCursorArea_(CRTData *crt);
void crt_setCursor_(CRTData *crt, uint8_t, uint8_t);
void crt_moveTo_(CRTData *crt, uint8_t, uint8_t);
void crt_advance_(CRTData *crt);

void crt_setBC_(CRTData *crt, uint8_t);
void crt_setFC_(CRTData *crt, uint8_t);

void crt_init(CRTData *crt) {
    write_mem((uint8_t *)crt, sizeof(CRTData), 0);

    crt->cursorOwner = PROC_ID_NONE;

    crt->screenBC = BC_WHITE;
    crt->screenFC = BC_BLACK;

    crt_setBC_(crt, BC_BLACK);
    crt_setFC_(crt, FC_WHITE);

    crt->outqWriter += write_string((char *)crt->outqBuf, CRT_OUTQ_LEN, "\x1b[2J");

    crt->lineBufLen += write_string((char *)crt->lineBuf, CRT_LINE_LIMIT, ">>");
    crt->userCursorPos = crt->lineBufLen;

    crt_setCursor_(crt, 1, 0); // force moveto_ to do something
    crt_moveTo_(crt, 0, crt->userCursorPos); // move to user typing location
}

uint8_t crt_hasOutByte(CRTData *crt) {
    crt_advance_(crt);
    return crt->outqReader < crt->outqWriter;
}

uint8_t crt_getOutByte(CRTData *crt) {
    uint8_t ret = 0;
    if (!crt_hasOutByte(crt)) {
        return 0;
    }

    ret = crt->outqBuf[crt->outqReader];

    ++(crt->outqReader);
    if (crt->outqReader == crt->outqWriter) {
        crt->outqWriter = 0;
        crt->outqReader = 0;
    }

    return ret;
}

void crt_advance_(CRTData *crt) {
    uint8_t mismatchPos = 0;

    // If we already have pending work, then no need to advance
    if(crt->outqReader < crt->outqWriter) {
        return;
    }

    // If the screen is showing more than our internal model, then we
    // need to clear the latter part of the screen.
    if (crt->screenBufLen > crt->lineBufLen) {
        crt_moveTo_(crt, 0, crt->lineBufLen);
        crt_setBC_(crt, BC_BLACK);
        crt_setFC_(crt, FC_WHITE);

        // clear from cursor to end of line
        crt->outqWriter += write_string(
            (char *)(crt->outqBuf + crt->outqWriter),
            CRT_OUTQ_LEN - crt->outqWriter,
            "\x1b[0K"
        );

        crt->screenBufLen = crt->lineBufLen;
        return;
    }

    // Otherwise, we know that the screen is showing less than or equal to
    // our internal model. We try to find the first mismatch.

    mismatchPos = crt->lastMismatchPos;
    while (mismatchPos < crt->screenBufLen && mismatchPos < crt->lineBufLen) {
        if (crt->screenBuf[mismatchPos] != crt->lineBuf[mismatchPos]) {
            break;
        }
        ++mismatchPos;
    }

    crt->lastMismatchPos = mismatchPos;

    // If the mismatch is before what our screen is showing, then we go to
    // that position, and output some characters (note that this will
    // simply overwrite any characters that may be on the screen right now)
    if (mismatchPos < crt->screenBufLen) {
        crt_moveTo_(crt, 0, mismatchPos);
        crt_setBC_(crt, BC_BLACK);
        crt_setFC_(crt, FC_WHITE);

        // NOTE(sanjay): this is an unsafe write
        crt->outqBuf[(crt->outqWriter)++] = crt->lineBuf[mismatchPos];
        crt->screenBuf[mismatchPos] = crt->lineBuf[mismatchPos];
        crt_setCursor_(crt, 0, mismatchPos+1);
        return;
    }

    // If there is no mismatch, but we are showing less than our internal
    // model, then show the next character in our internal model.
    if (
        mismatchPos == crt->screenBufLen &&
        crt->screenBufLen < crt->lineBufLen
    ) {
        crt_moveTo_(crt, 0, mismatchPos);
        crt_setBC_(crt, BC_BLACK);
        crt_setFC_(crt, FC_WHITE);

        // NOTE(sanjay): this is an unsafe write
        crt->outqBuf[(crt->outqWriter)++] = crt->lineBuf[mismatchPos];
        ++(crt->screenBufLen);
        crt->screenBuf[mismatchPos] = crt->lineBuf[mismatchPos];
        crt_setCursor_(crt, 0, mismatchPos+1);
        return;
    }

    // We now know that screenBufLen == lineBufLen == mismatchPos
    // so screen data on the user line exactly matches our internal model.
    // We now move on to process output.

    while (crt->envqHead != NULL) {
        Envelope *nextEnv = crt->envqHead;
        uint8_t nextByte = 0;

        // If we see a null byte in our current envelope, or we've fallen
        // off the end of this envelope, give up on this envelope.
        if (
            crt->readIndex >= MESSAGEDATA_SIZE_BYTES ||
            nextEnv->messageData[crt->readIndex] == '\0'
        ) {
            // Dequeue from envQ
            crt->envqHead = nextEnv->next;
            if (crt->envqHead == NULL) {
                crt->envqTail = NULL;
            }

            crt->readIndex = 0;

            // Put on free list
            nextEnv->next = crt->freeList;
            crt->freeList = nextEnv;

            // Reset colors
            crt->procBC = BC_BLACK;
            crt->procFC = FC_WHITE;
            continue;
        }

        nextByte = nextEnv->messageData[crt->readIndex];
        ++(crt->readIndex);

        if (nextByte >= FOREGROUND_COLOR_BASE && nextByte < FC_UPPER_BOUND) {
            crt->procFC = nextByte;
            continue;
        } else if (
            nextByte >= BACKGROUND_COLOR_BASE &&
            nextByte < BC_UPPER_BOUND
        ) {
            crt->procBC = nextByte;
            continue;
        }


        // The only special character we support for now is \n, any other
        // special characters are simply skipped.
        if (!is_printable(nextByte) && nextByte != '\n') {
            continue;
        }

        // If we just switched to a new cursorOwner and we have some content
        // on the process line, then scroll the process area up, move to
        // the beginning of the process line, and output this byte.
        if (nextEnv->srcPid != crt->cursorOwner) {
            crt->cursorOwner = nextEnv->srcPid;

            // If they had content on this line, then scroll to the next line
            if (crt->procCursorPos > 0) {
                crt_scrollCursorArea_(crt);
            }
        }

        // If we are printing a newline, then just scroll this process area.
        if (nextByte == '\n' && crt->procCursorPos > 0) {
            crt_scrollCursorArea_(crt);
            return;
        }

        // If the process has output more than 80 characters, truncate.
        if (crt->procCursorPos >= CRT_LINE_LIMIT) {
            continue;
        }

        // Otherwise, merely move to the correct location, and output the
        // next byte.
        crt_moveTo_(crt, 1, crt->procCursorPos);
        crt_setCursor_(crt, 1, (crt->procCursorPos)+1);
        ++(crt->procCursorPos);
        crt_setBC_(crt, crt->procBC);
        crt_setFC_(crt, crt->procFC);

        // Enqueue this character
        // NOTE(sanjay): this is an unsafe write
        crt->outqBuf[(crt->outqWriter)++] = nextByte;
        return;
    }

    crt_moveTo_(crt, 0, crt->userCursorPos);
}

void crt_scrollCursorArea_(CRTData *crt) {
    crt_moveTo_(crt, 0, 0); // move to beginning of user line
    crt_setBC_(crt, BC_BLACK);
    crt_setFC_(crt, FC_WHITE);

    // clear entire line and scroll up one line
    crt->outqWriter += write_string(
        (char *)(crt->outqBuf + crt->outqWriter),
        CRT_OUTQ_LEN - crt->outqWriter,
        "\x1b[2K\x1b[1S"
    );

    crt_setCursor_(crt, 1, 0);
    crt_moveTo_(crt, 0, 0);

    crt->procCursorPos = 0;
    crt->screenBufLen = 0; // This will cause the user line to be recopied.
    crt->lastMismatchPos = 0;
}

void crt_moveTo_(CRTData *crt, uint8_t isProcLine, uint8_t pos) {
    uint8_t desired = 0;

    // First, make sure these are valid.
    if (isProcLine) {
        isProcLine = 1;
    }
    pos = pos & 0x7F;

    // Then, see if we are already there.
    desired = (isProcLine << 7) | pos;
    if (crt->screenCursorPos == desired) {
        return;
    }

    // First, we move it to the right column, at the bottom of the screen
    crt->outqWriter += write_string(
        (char *)(crt->outqBuf + crt->outqWriter),
        CRT_OUTQ_LEN - crt->outqWriter,
        "\x1b[10000;"
    );
    crt->outqWriter += write_uint32(
        (char *)(crt->outqBuf + crt->outqWriter),
        CRT_OUTQ_LEN - crt->outqWriter,
        pos+1,
        0
    );
    crt->outqWriter += write_string(
        (char *)(crt->outqBuf + crt->outqWriter),
        CRT_OUTQ_LEN - crt->outqWriter,
        "H"
    );

    // Then, we move it up a row if necessary
    if (isProcLine == 1) {
        crt->outqWriter += write_string(
            (char *)(crt->outqBuf + crt->outqWriter),
            CRT_OUTQ_LEN - crt->outqWriter,
            "\x1b[1A"
        );
    }

    crt->screenCursorPos = desired;
}

void crt_setCursor_(CRTData *crt, uint8_t isProcLine, uint8_t pos) {
    // First, make sure these are valid.
    if (isProcLine) {
        isProcLine = 1;
    }
    pos = pos & 0x7F;
    crt->screenCursorPos = (isProcLine << 7) | pos;
}

void crt_pushProcEnv(CRTData *crt, Envelope *env) {
    if (crt->envqHead == NULL) {
        crt->envqHead = env;
        crt->envqTail = env;
    } else {
        env->next = NULL;
        crt->envqTail->next = env;
        crt->envqTail = env;
    }
}

void crt_pushUserByte(CRTData *crt, uint8_t c) {
    // First, handle backspaces
    if (c == 0x7F) {
        uint8_t i = 0;

        // If the line is empty, or user is at beginning of string, give up
        if (crt->lineBufLen <= PROMPT_LEN || crt->userCursorPos <= PROMPT_LEN) {
            return;
        }

        --(crt->lineBufLen);
        --(crt->userCursorPos);
        for (i = crt->userCursorPos; i < crt->lineBufLen; ++i) {
            crt->lineBuf[i] = crt->lineBuf[i+1];
        }

        crt->lastMismatchPos = 0;

        return;
    }

    // TODO(sanjay): all the rest of our code works allowing userCursorPos
    // to range from 0 to lineBufLen, but as it stands, userCursorPos can
    // only be equal to lineBufLen. We should handle the keycodes for
    // moving left and right, to allow the cursor to move into the middle
    // of lines.

    if (!is_printable(c)) {
        return;
    }

    if (crt->lineBufLen >= CRT_LINE_LIMIT) {
        return;
    }

    // First, if we're not at the end, make space by shifting everything down
    if (crt->userCursorPos < crt->lineBufLen) {
        uint8_t i = 0;
        for (i = crt->lineBufLen; i > crt->userCursorPos; --i) {
            crt->lineBuf[i] = crt->lineBuf[i-1];
        }
    }

    crt->lineBuf[crt->userCursorPos] = c;
    ++(crt->userCursorPos);
    ++(crt->lineBufLen);
}

void crt_setBC_(CRTData *crt, uint8_t bc) {
    if (crt->screenBC == bc) {
        return;
    }

    crt->screenBC = bc;
    crt->outqWriter += write_string(
        (char *)(crt->outqBuf + crt->outqWriter),
        CRT_OUTQ_LEN - crt->outqWriter,
        "\x1b["
    );
    crt->outqWriter += write_uint32(
        (char *)(crt->outqBuf + crt->outqWriter),
        CRT_OUTQ_LEN - crt->outqWriter,
        bc - BACKGROUND_COLOR_BASE + 40,
        0
    );
    crt->outqWriter += write_string(
        (char *)(crt->outqBuf + crt->outqWriter),
        CRT_OUTQ_LEN - crt->outqWriter,
        "m"
    );
}

void crt_setFC_(CRTData *crt, uint8_t fc) {
    if (crt->screenFC == fc) {
        return;
    }

    crt->screenFC = fc;
    crt->outqWriter += write_string(
        (char *)(crt->outqBuf + crt->outqWriter),
        CRT_OUTQ_LEN - crt->outqWriter,
        "\x1b["
    );
    crt->outqWriter += write_uint32(
        (char *)(crt->outqBuf + crt->outqWriter),
        CRT_OUTQ_LEN - crt->outqWriter,
        fc - FOREGROUND_COLOR_BASE + 30,
        0
    );
    crt->outqWriter += write_string(
        (char *)(crt->outqBuf + crt->outqWriter),
        CRT_OUTQ_LEN - crt->outqWriter,
        "m"
    );
}

uint8_t crt_hasFreeEnv(CRTData *crt) {
    return crt->freeList != NULL;
}

Envelope *crt_getFreeEnv(CRTData *crt) {
    Envelope *temp = crt->freeList;
    if (temp != NULL) {
        crt->freeList = temp->next;
    }
    temp->next = NULL;
    return temp;
}
