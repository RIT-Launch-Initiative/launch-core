/*
 * @file slip.h
 * @brief SLIP protocol implementation
 * @see RFC 1055 https://www.rfc-editor.org/rfc/pdfrfc/rfc1055.txt.pdf
 *               https://en.wikipedia.org/wiki/Serial_Line_Internet_Protocol#Description
 * @author Nate Aquino
 */

#ifndef SLIP_H
#define SLIP_H

/*
 * @brief SLIP protocol special characters
 */

/* Frame End */
#define END 0xC0
/* Frame Escape */
#define ESC 0xDB
/* Transposed Frame End, sent with a prepended END
   if and only if the END byte occurs in the data. */
#define ESC_END 0xDC
/* Transposed Frame Escape, sent with a prepended ESC
   if and only if the ESC byte occurs in the data. */
#define ESC_ESC 0xDD

// todo: integrate
#define send_char(c)
#define recv_char() 0

/*
 * Send a packet p of length len thru the port.
 * @param p the packet to send
 * @param len the length of the packet
 */
void send_packet(char* p, int len) {
    send_char(end);
    while (len--) {
        switch (*p) {
            case END:
                send_char(ESC);
                send_char(ESC_END);
                break;
            case ESC:
                send_char(ESC);
                send_char(ESC_ESC);
                break;
            default:
                send_char(*p);
        }
        p++;
    }
    send_char(END);
}

/*
 * Recieve up to len bytes into p.
 * @param p the buffer to store the packet
 * @param len the length of the buffer
 * @returns the number of bytes received
 */
int recv_packet(char* p, int len) {
    char c;
    int recv = 0;
    for (;;) {
        c = recv_char();
        switch (c) {
            case END:
                if (recv)
                    return recv;
                else
                    break;
            case ESC:
                c = recv_char();
                switch (c) {
                    case ESC_END:
                        c = END;
                        break;
                    case ESC_ESC:
                        c = ESC;
                        break;
                }
            default:
                if (recv < len)
                    p[recv++] = c;
        }
    }
}

#endif  // !SLIP_H