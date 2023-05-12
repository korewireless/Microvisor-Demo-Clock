/*
 * cellular::i2c
 *
 * @version     1.0.0
 * @author      smittytone
 * @copyright   2021
 * @licence     MIT
 *
 */
#ifndef _I2C_HEADER_
#define _I2C_HEADER_


/*
 * CONSTANTS
 */
#define     I2C_GPIO_BANK           GPIOB


/*
 * PROTOTYPES
 */
namespace I2C {
    
    void        setup(void);
    bool        check(uint8_t addr);
    void        write_byte(uint8_t address, uint8_t byte);
    void        write_block(uint8_t address, uint8_t *data, uint8_t count);
}


#endif  // _I2C_HEADER_
