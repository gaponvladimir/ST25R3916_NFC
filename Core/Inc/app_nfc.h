/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_NFC_H
#define APP_NFC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/** @addtogroup PollingTagDetect
 *  @{
 */


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
/** @defgroup PTD_Demo_Exported_Functions
 *  @{
 */
bool app_nfc_Ini( void );
void app_nfc_Cycle(void);
void app_nfc_start_discovery(void);
void app_nfc_stop_discovery(void);
int app_nfc_get_ATR(uint8_t *atr);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* APP_NFC_H */

