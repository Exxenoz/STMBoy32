#ifndef SDC_CONFIG_H
#define SDC_CONFIG_H

#define SDC_SPIx                        SPI3

#define SDC_SPIx_CS_GPIO_PORT           GPIOA
#define SDC_SPIx_CS_PIN                 GPIO_PIN_15

#define SDC_SPIx_SCK_GPIO_PORT          GPIOC
#define SDC_SPIx_SCK_PIN                GPIO_PIN_10
#define SDC_SPIx_SCK_AF                 GPIO_AF6_SPI3

#define SDC_SPIx_MISO_MOSI_GPIO_PORT    GPIOC
#define SDC_SPIx_MISO_PIN               GPIO_PIN_11
#define SDC_SPIx_MOSI_PIN               GPIO_PIN_12
#define SDC_SPIx_MISO_MOSI_AF           GPIO_AF6_SPI3

#define SPIx__CS_HIGH()                 HAL_GPIO_WritePin(SDC_SPIx_CS_GPIO_PORT, SDC_SPIx_CS_PIN, GPIO_PIN_SET)
#define SPIx__CS_LOW()                  HAL_GPIO_WritePin(SDC_SPIx_CS_GPIO_PORT, SDC_SPIx_CS_PIN, GPIO_PIN_RESET)


#define SD_CS_GPIO_PORT                 GPIOA
#define SD_CS_PIN                       GPIO_PIN_15

#define SD_CS_LOW()                     HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_PIN_RESET)
#define SD_CS_HIGH()                    HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_PIN_SET)


/* Maximum Timeout values for flags waiting loops.
   These timeouts are not based on accurate values, they just guarantee that the application will not remain stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application conditions (interrupts routines ...). */
   
#define SDC_SPIx_TIMEOUT_MAX            1000
#define SDC_SPIx_TIMEOUT                SDC_SPIx_TIMEOUT_MAX

#endif //SDC_CONFIG_H
