#ifndef SDC_CONFIG_H
#define SDC_CONFIG_H

#define SDC_SPI_MODULE_ENABLED

/*############################### SPI #######################################*/
#ifdef SDC_SPI_MODULE_ENABLED

#define SDC_SPIx                                     SPI3
#define SDC_SPIx_CLK_ENABLE()                        __HAL_RCC_SPI3_CLK_ENABLE()

#define SDC_SPIx_SCK_AF                              GPIO_AF5_SPI3
#define SDC_SPIx_SCK_GPIO_PORT                       GPIOC
#define SDC_SPIx_SCK_PIN                             GPIO_PIN_10
#define SDC_SPIx_SCK_GPIO_CLK_ENABLE()               __HAL_RCC_GPIOC_CLK_ENABLE()
#define SDC_SPIx_SCK_GPIO_CLK_DISABLE()              //__HAL_RCC_GPIOC_CLK_DISABLE()

#define SDC_SPIx_MISO_MOSI_AF                        GPIO_AF5_SPI3
#define SDC_SPIx_MISO_MOSI_GPIO_PORT                 GPIOC
#define SDC_SPIx_MISO_MOSI_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOC_CLK_ENABLE()
#define SDC_SPIx_MISO_MOSI_GPIO_CLK_DISABLE()        //__HAL_RCC_GPIOC_CLK_DISABLE()
#define SDC_SPIx_MISO_PIN                            GPIO_PIN_11
#define SDC_SPIx_MOSI_PIN                            GPIO_PIN_12
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define SDC_SPIx_TIMEOUT_MAX                   1000

#define SDC_SPIx_CS_GPIO_PORT                        GPIOA
#define SDC_SPIx_CS_PIN                              GPIO_PIN_15
#define SDC_SPIx_CS_GPIO_CLK_ENABLE()                __HAL_RCC_GPIOA_CLK_ENABLE()
#define SDC_SPIx_CS_GPIO_CLK_DISABLE()               //__HAL_RCC_GPIOA_CLK_DISABLE()

#define SPIx__CS_LOW()       HAL_GPIO_WritePin(SDC_SPIx_CS_GPIO_PORT, SDC_SPIx_CS_PIN, GPIO_PIN_RESET)
#define SPIx__CS_HIGH()      HAL_GPIO_WritePin(SDC_SPIx_CS_GPIO_PORT, SDC_SPIx_CS_PIN, GPIO_PIN_SET)

/**
  * @brief  SD Control Lines management
  */
#define SD_CS_LOW()       HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_PIN_RESET)
#define SD_CS_HIGH()      HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_PIN, GPIO_PIN_SET)

/**
  * @brief  SD Control Interface pins (shield D4)
  */
#define SD_CS_PIN                                 GPIO_PIN_15
#define SD_CS_GPIO_PORT                           GPIOA
#define SD_CS_GPIO_CLK_ENABLE()                 __HAL_RCC_GPIOA_CLK_ENABLE()
#define SD_CS_GPIO_CLK_DISABLE()                //__HAL_RCC_GPIOA_CLK_DISABLE()

/*############################### SDIO #######################################*/
#else

#define SDC_SDIO_NVIC_CHANNEL SDIO_IRQn
#define SDC_DMA_NVIC_CHANNEL  SD_SDIO_DMA_IRQn

#define SDC_DAT0_PORT         GPIOC
#define SDC_DAT0_PIN          GPIO_PIN_8
#define SDC_DAT0_PIN_SOURCE   GPIO_PinSource8

#define SDC_DAT1_PORT         GPIOC
#define SDC_DAT1_PIN          GPIO_PIN_9
#define SDC_DAT1_PIN_SOURCE   GPIO_PinSource9

#define SDC_DAT2_PORT         GPIOC
#define SDC_DAT2_PIN          GPIO_PIN_10
#define SDC_DAT2_PIN_SOURCE   GPIO_PinSource10

#define SDC_DAT3_PORT         GPIOC
#define SDC_DAT3_PIN          GPIO_PIN_11
#define SDC_DAT3_PIN_SOURCE   GPIO_PinSource11

#define SDC_CLK_PORT          GPIOC
#define SDC_CLK_PIN           GPIO_PIN_12
#define SDC_CLK_PIN_SOURCE    GPIO_PinSource12
                              
#define SDC_CMD_PORT          GPIOD
#define SDC_CMD_PIN           GPIO_PIN_2
#define SDC_CMD_PIN_SOURCE    GPIO_PinSource2

#endif

#endif //SDC_CONFIG_H
