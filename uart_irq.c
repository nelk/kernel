/**
 * @brief: uart_irq.c
 * @author: NXP Semiconductors
 * @author: Y. Huang
 * @date: 2013/02/12
 */

#include <LPC17xx.h>

#include "coq.h"
#include "helpers.h"
#include "kernel_types.h"
#include "rtx.h"
#include "uart.h"

extern MemInfo gMemInfo;
extern ProcInfo gProcInfo;

/**
 * @brief: initialize the n_uart
 * NOTES: only fully supports uart0 so far, but can be easily extended
 *        to other uarts.
 *        The step number in the comments matches the item number
 *        in Section 14.1 on pg 298 of LPC17xx_UM
 */
int uart_init(int n_uart) {

    LPC_UART_TypeDef *pUart;

    if (n_uart ==0 ) {
        /*
           Steps 1 & 2: system control configuration.
           Under CMSIS, system_LPC17xx.c does these two steps

           -----------------------------------------------------
           Step 1: Power control configuration.
           See table 46 pg63 in LPC17xx_UM
           -----------------------------------------------------
           Enable UART0 power, this is the default setting
           done in system_LPC17xx.c under CMSIS.
           Enclose the code for your refrence
        //LPC_SC->PCONP |= BIT(3);

        -----------------------------------------------------
        Step2: Select the clock source.
        Default PCLK=CCLK/4 , where CCLK = 100MHZ.
        See tables 40 & 42 on pg56-57 in LPC17xx_UM.
        -----------------------------------------------------
        Check the PLL0 configuration to see how XTAL=12.0MHZ
        gets to CCLK=100MHZin system_LPC17xx.c file.
        PCLK = CCLK/4, default setting after reset.
        Enclose the code for your reference
        //LPC_SC->PCLKSEL0 &= ~(BIT(7)|BIT(6));

        -----------------------------------------------------
        Step 5: Pin Ctrl Block configuration for TXD and RXD
        See Table 79 on pg108 in LPC17xx_UM.
        -----------------------------------------------------
        Note this is done before Steps3-4 for coding purpose.
        */

        /* Pin P0.2 used as TXD0 (Com0) */
        LPC_PINCON->PINSEL0 |= (1 << 4);

        /* Pin P0.3 used as RXD0 (Com0) */
        LPC_PINCON->PINSEL0 |= (1 << 6);

        pUart = (LPC_UART_TypeDef *) LPC_UART0;

    } else if (n_uart == 1) {

        /* see Table 79 on pg108 in LPC17xx_UM */
        /* Pin P2.0 used as TXD1 (Com1) */
        LPC_PINCON->PINSEL0 |= (2 << 0);

        /* Pin P2.1 used as RXD1 (Com1) */
        LPC_PINCON->PINSEL0 |= (2 << 2);

        pUart = (LPC_UART_TypeDef *) LPC_UART1;

    } else {
        return 1; /* not supported yet */
    }

    /*
       -----------------------------------------------------
       Step 3: Transmission Configuration.
       See section 14.4.12.1 pg313-315 in LPC17xx_UM
       for baud rate calculation.
       -----------------------------------------------------
       */

    /* Step 3a: DLAB=1, 8N1 */
    pUart->LCR = UART_8N1; /* see uart.h file */

    /* Step 3b: 115200 baud rate @ 25.0 MHZ PCLK */
    pUart->DLM = 0; /* see table 274, pg302 in LPC17xx_UM */
    pUart->DLL = 9; /* see table 273, pg302 in LPC17xx_UM */

    /* FR = 1.507 ~ 1/2, DivAddVal = 1, MulVal = 2
       FR = 1.507 = 25MHZ/(16*9*115200)
       see table 285 on pg312 in LPC_17xxUM
       */
    pUart->FDR = 0x21;



    /*
       -----------------------------------------------------
       Step 4: FIFO setup.
       see table 278 on pg305 in LPC17xx_UM
       -----------------------------------------------------
       enable Rx and Tx FIFOs, clear Rx and Tx FIFOs
       Trigger level 0 (1 char per interrupt)
       */

    pUart->FCR = 0x07;

    /* Step 5 was done between step 2 and step 4 a few lines above */

    /*
       -----------------------------------------------------
       Step 6 Interrupt setting and enabling
       -----------------------------------------------------
       */
    /* Step 6a:
       Enable interrupt bit(s) wihtin the specific peripheral register.
       Interrupt Sources Setting: RBR, THRE or RX Line Stats
       See Table 50 on pg73 in LPC17xx_UM for all possible UART0 interrupt sources
       See Table 275 on pg 302 in LPC17xx_UM for IER setting
       */
    /* disable the Divisior Latch Access Bit DLAB=0 */
    pUart->LCR &= ~(BIT(7));

    pUart->IER = IER_RBR | IER_THRE | IER_RLS;

    /* Step 6b: enable the UART interrupt from the system level */
    NVIC_EnableIRQ(UART0_IRQn); /* CMSIS function */

    return 0;
}

/**
 * @brief: use CMSIS ISR for UART0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine.
 *       The actual c_UART0_IRQHandler does the rest of irq handling
 */
__asm void UART0_IRQHandler(void) {
    PRESERVE8
    IMPORT c_UART0_IRQHandler
    PUSH{r4-r11, lr}
    BL c_UART0_IRQHandler
    POP{r4-r11, pc}
}

void uart_receive_char_isr(ProcInfo *procInfo, char new_char) {
    int localWriter = procInfo->writeIndex;
    if ((localWriter + 1) % UART_IN_BUF_SIZE == procInfo->readIndex) {
        procInfo->inputBufOverflow = 1;
        return;
    }
    procInfo->inputBuf[localWriter] = new_char;
    procInfo->writeIndex = (localWriter + 1) % UART_IN_BUF_SIZE;
}

void uart_send_char_isr(ProcInfo *procInfo) {
    LPC_UART_TypeDef *uart = (LPC_UART_TypeDef *)LPC_UART0;
    uint32_t localReader = procInfo->outReader;
    uint32_t localWriter = procInfo->outWriter;
    uint8_t sent = 0;

    if (!(uart->LSR & LSR_THRE)) {
        return;
    }

    while (localReader != localWriter && sent < UART_OUTPUT_BUFSIZE) {
        uart->THR = procInfo->outputBuf[localReader];
        localReader = (localReader+1) % OUTPUT_BUFSIZE;
        sent++;
    }
    procInfo->outReader = localReader;
}

void c_UART0_IRQHandler(void) {
    uint8_t IIR_IntId;      /* Interrupt ID from IIR */
    uint8_t LSR_Val;        /* LSR Value             */
    uint8_t dummy = dummy;  /* to clear interrupt upon LSR error */
    LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;

    /* Reading IIR automatically acknowledges the interrupt */
    IIR_IntId = (pUart->IIR) >> 1 ; /* skip pending bit in IIR */

    if (IIR_IntId & IIR_RDA) { /* Receive Data Available */
        /* read UART. Read RBR will clear the interrupt */
        uart_receive_char_isr(&gProcInfo, pUart->RBR);
    } else if (IIR_IntId & IIR_THRE) {
        /* THRE Interrupt, transmit holding register empty*/
        // NOTE(sanjay): Make sure that this is contant time, we are in an ISR.
        if (gProcInfo.outLock == 0) {
            uart_send_char_isr(&gProcInfo);
        }

        LSR_Val = pUart->LSR;
    } else if (IIR_IntId & IIR_RLS) {
        LSR_Val = pUart->LSR;
        if (LSR_Val  & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) ) {
            /* There are errors or break interrupt
               Read LSR will clear the interrupt
               Dummy read on RX to clear interrupt, then bail out
               */
            dummy = pUart->RBR;
            return; /* error occurs, return */
        }
        /* If no error on RLS, normal ready, save into the data buffer.
Note: read RBR will clear the interrupt
*/
        if (LSR_Val & LSR_RDR) { /* Receive Data Ready */
            /* read from the uart */
            uart_receive_char_isr(&gProcInfo, pUart->RBR);
        }
    } else { /* IIR_CTI and reserved combination are not implemented */
        return;
    }
}

void crt_proc(void) {
    Envelope *temp = NULL;
		uint32_t localWriter = 0;
    uint32_t localReader = 0;
    while (1) {
        Envelope *nextMsg = NULL;

        while (gProcInfo.coq.toFree != NULL) {
            temp = gProcInfo.coq.toFree;
            gProcInfo.coq.toFree = gProcInfo.coq.toFree->next;
            release_memory_block((void*)temp);
        }

        nextMsg = (Envelope *)receive_message(NULL);
        if (nextMsg == NULL) {
            continue;
        }

        if (nextMsg->srcPid != CRT_PID) {
            pushEnvelope(&(gProcInfo.coq), nextMsg);
        } else {
            gProcInfo.uartOutputEnv = nextMsg;
        }

        localWriter = gProcInfo.outWriter;
        localReader = gProcInfo.outReader;
        while (
            ((localWriter + 1) % OUTPUT_BUFSIZE != localReader) &&
            (hasData(&(gProcInfo.coq), &gMemInfo))
        ) {
            gProcInfo.outputBuf[localWriter] = getData(&(gProcInfo.coq), &gMemInfo);
            localWriter = (localWriter + 1) % OUTPUT_BUFSIZE;
        }
        gProcInfo.outWriter = localWriter;

        gProcInfo.outLock = 1;
        uart_send_char_isr(&gProcInfo);
        gProcInfo.outLock = 0;
    }
}

char toLowerAndIsLetter(char c) {
    if (c >= 'a' && c <= 'z') {
        return c;
    } else if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    return '\0';
}

size_t writePCBState(char *buf, size_t bufLen, ProcState state) {
    switch (state) {
        case BLOCKED_MEMORY:
            return write_string(buf, bufLen, "Blocked on memory");
        case BLOCKED_MESSAGE:
            return write_string(buf, bufLen, "Blocked on message");
        case NEW:
            return write_string(buf, bufLen, "New");
        case READY:
            return write_string(buf, bufLen, "Ready");
        case RUNNING:
            return write_string(buf, bufLen, "Running");
        default:
            break;
    }

    return write_string(buf, bufLen, "???");
}

size_t writeProcessInfo(char *buf, size_t bufLen, PCB *pcb) {
    size_t i = 0;
    i += write_ansi_escape(buf+i, bufLen-i, 41);
    i += write_uint32(buf+i, bufLen-i, pcb->pid, 0);
    i += write_string(buf+i, bufLen-i, "$ Priority=");
    i += write_uint32(buf+i, bufLen-i, pcb->priority, 0);
    i += write_string(buf+i, bufLen-i, ", Status=");
    i += writePCBState(buf+i, bufLen-i, pcb->state);
    i += write_ansi_escape(buf+i, bufLen-i, 0);
    i += write_string(buf+i, bufLen-i, "\r\n");
    return i;
}

void uart_keyboard_proc(void) {
    Envelope *message = NULL;
    Envelope *messageCopy = NULL;
    uint32_t idx = 0;
    ProcId registry['z' - 'a' + 1] = {0};
    ProcId destPid = 0;
    char c = '\0';
    uint8_t reject = 1;

    while (1) {
        message = receive_message(NULL);
        if (message == NULL) {
            continue;
        }

        // If the message is not "from ourself", that means we should
        // register this character with the associated pid
        if (message->srcPid != KEYBOARD_PID) {
            c = toLowerAndIsLetter(message->messageData[0]);
            if ('a' <= c && c <= 'z') {
                registry[c - 'a'] = message->srcPid;
            }

            release_memory_block(message);
            message = NULL;
            continue;
        } else if (message->messageData[0] == SHOW_DEBUG_PROCESSES) {
            Envelope *tempEnvelope = NULL;
            uint8_t i = 0;
            uint8_t bufLen = 0;

            for (; i < NUM_PROCS; i++) {
                uint32_t location = 0;
                PCB *pcb = &(gProcInfo.processes[i]);

                // Check if this is an unused process slot
                if (*(pcb->startLoc) == 0) {
                    continue;
                }

                tempEnvelope = (Envelope *)request_memory_block();
                location += writeProcessInfo(
                    tempEnvelope->messageData,
                    MESSAGEDATA_SIZE_BYTES - 1, // -1 for null byte
                    pcb
                );
                tempEnvelope->messageData[location++] = '\0';
                send_message(CRT_PID, tempEnvelope);
                tempEnvelope = NULL;
            }

            i = 0;
            bufLen = MESSAGEDATA_SIZE_BYTES-1; // -1 for null byte

            i += write_ansi_escape(message->messageData+i, bufLen-i, 41);
            i += write_string(message->messageData+i, bufLen-i, "used mem = ");
            i += write_uint32(
                message->messageData+i,
                bufLen-i,
                (gMemInfo.numSuccessfulAllocs-gMemInfo.numFreeCalls)*128,
                2
            );
            i += write_string(message->messageData+i, bufLen-i, " bytes");
            i += write_ansi_escape(message->messageData+i, 0);
            i += write_string(message->messageData+i, bufLen-i, "\r\n");
            message->messageData[i++] = '\0';
            send_message(CRT_PID, message);
            message = NULL;
            continue;
        }

        // If it is "from ourself", then we send a message to the
        // registered processes.
        c = message->messageData[0];
        reject = 1;
        if (c == '%') {
            c = toLowerAndIsLetter(message->messageData[1]);
            if (c != '\0') {
                destPid = registry[c - 'a'];
                if (destPid != 0) {
                    reject = 0;
                }
            }
        }

        if (reject) {
            // Set copy for crt proc this message (since original is not being sent anywhere)
            messageCopy = message;
            message = NULL;
        } else {
            // Create copy to send to crt proc
            messageCopy = (Envelope *)request_memory_block();
            messageCopy->srcPid = message->srcPid;
            messageCopy->dstPid = message->dstPid;
            messageCopy->messageType = message->messageType;
            messageCopy->sendTime = message->sendTime;
            for (idx = 0; idx < MESSAGEDATA_SIZE_BYTES; ++idx) {
                messageCopy->messageData[idx] = message->messageData[idx];
            }

            // Send the message to the proc that registered with this character
            send_message(destPid, message);
            message = NULL;
        }

        // Send copy of message to CRT proc
        if (messageCopy != NULL) {
            send_message(CRT_PID, messageCopy);
            messageCopy = NULL;
        }
    }
}

