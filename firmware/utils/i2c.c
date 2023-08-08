#include <stdint.h>
#include <stddef.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "i2c.h"

// mapping of TWI io devs --> i2c_dev_t objects
i2c_dev_t *devs[2];

void i2c_init(i2c_dev_t *dev, TWI_t *io, uint8_t bauddiv){
    if(io == &TWI0) {
        devs[0] = dev;
    } else if(io == &TWI1){
        devs[1] = dev;
    }

    dev->io = io;
    dev->status = I2C_IDLE;

    io->CTRLA = 0;
    io->MBAUD = bauddiv;
    io->MSTATUS = TWI_BUSSTATE_IDLE_gc | TWI_RIF_bm | TWI_WIF_bm | TWI_CLKHOLD_bm | TWI_ARBLOST_bm | TWI_BUSERR_bm;
    io->MCTRLA = TWI_ENABLE_bm | TWI_RIEN_bm | TWI_WIEN_bm;
}

void i2c_transfer_start(i2c_dev_t *dev, i2c_package_t *pkg, void (*callback)(i2c_status_t result)){
    if(i2c_transfer_status(dev) == I2C_BUSY) return;

    dev->pkg = (i2c_package_t*)pkg;
    dev->idx = 0;
    dev->callback = callback;
    dev->status = I2C_BUSY;

    if((pkg->addr_len == 0) && (pkg->data_len == 0)){
        // No payload. Issuing a "quick-command"
        dev->io->MCTRLA |= TWI_QCEN_bm;
    }else{
        dev->io->MCTRLA &= ~TWI_QCEN_bm;
    }

    if(pkg->addr_len > 0){
        // Need to write register address first.
        dev->sending_addr = true;
        dev->io->MADDR = (pkg->slave_addr << 1);
    } else {
        // No address stage. Data transfer only
        dev->sending_addr = false;

        if(pkg->read){
            dev->io->MADDR = (pkg->slave_addr << 1) | 0x01;
        }else{
            dev->io->MADDR = (pkg->slave_addr << 1);
        }
    }

}

i2c_status_t i2c_transfer_status(i2c_dev_t *dev){
    i2c_status_t status;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        status = dev->status;
    }

    return(status);
}

static void twim_isr(i2c_dev_t *dev){
    uint8_t const status = dev->io->MSTATUS;

    // Clear all flags
    dev->io->MSTATUS = TWI_RIF_bm | TWI_WIF_bm | TWI_CLKHOLD_bm | TWI_ARBLOST_bm | TWI_BUSERR_bm;

    if(dev->pkg == NULL) return;

    if(status & (TWI_ARBLOST_bm | TWI_BUSERR_bm | TWI_RXACK_bm)){
        // Error occurred
        dev->io->MCTRLB = TWI_MCMD_STOP_gc | TWI_ACKACT_NACK_gc;
        dev->status = I2C_FAILED;
        if(dev->callback){
            dev->callback(I2C_FAILED);
        }


    } else if(status & TWI_WIF_bm) { //--------------------------
        // Write interrupt. Ready to send another byte

        if(dev->sending_addr){
            if(dev->idx < dev->pkg->addr_len) {
                // Send Byte
                dev->io->MDATA = dev->pkg->addr[dev->idx];
                dev->idx++;
            } else {
                // Done sending address
                dev->sending_addr = false;
                dev->idx = 0;
                if(dev->pkg->read){
                    // Re-start condition to do a read
                    dev->io->MADDR |= 0x01;
                    return;
                }
            }
        }

        if(!dev->sending_addr){
            if(dev->idx < dev->pkg->data_len) {
                // Send Byte
                dev->io->MDATA = dev->pkg->data[dev->idx];
                dev->idx++;
            } else {
                // Done with transfer
                dev->io->MCTRLB = TWI_MCMD_STOP_gc | TWI_ACKACT_ACK_gc;
                dev->status = I2C_IDLE;
                if(dev->callback){
                    dev->callback(I2C_IDLE);
                }
            }
        }


    } else if(status & TWI_RIF_bm) { //--------------------------
        // Read interrupt. Finished receiving a byte
        if(dev->idx < dev->pkg->data_len){
            dev->pkg->data[dev->idx] = dev->io->MDATA;
            dev->idx++;
        }else{
            // Nowhere to put rd data. Stop
            dev->io->MCTRLB = TWI_MCMD_STOP_gc | TWI_ACKACT_ACK_gc;
            dev->status = I2C_IDLE;
            if(dev->callback){
                dev->callback(I2C_IDLE);
            }
        }

        // Receive another byte?
        if(dev->idx < dev->pkg->data_len){
            dev->io->MCTRLB = TWI_MCMD_RECVTRANS_gc;
        }else{
            // Done
            dev->io->MCTRLB = TWI_MCMD_STOP_gc | TWI_ACKACT_NACK_gc;
            dev->status = I2C_IDLE;
            if(dev->callback){
                dev->callback(I2C_IDLE);
            }
        }

    }
}


ISR(TWI0_TWIM_vect) {twim_isr(devs[0]);}
ISR(TWI1_TWIM_vect) {twim_isr(devs[1]);}
