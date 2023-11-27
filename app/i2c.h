/*
 * cellular::i2c
 *
 * @version     0.1.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#ifndef _I2C_HEADER_
#define _I2C_HEADER_


/*
 * NAMESPACES
 */
namespace I2C {

    void        setup(uint32_t address);
    void        writeByte(uint8_t address, uint8_t byte);
    void        writeBlock(uint8_t address, uint8_t *data, uint8_t count);
}


#endif  // _I2C_HEADER_
