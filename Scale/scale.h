/**
 * @file scale.c
 *
 * @brief Scale controller
 *
 * @author Benjamin Balga
 */

#ifndef _SCALE_H_
#define _SCALE_H_


/**
 * @brief Setup the Scale I/O layer. Must be called first WITH ROOT RIGHTS !!
 *
 * @return true if scale I/O layer successfully initialized, false otherwise.
 */
bool bScale_setup();


/**
 * @brief Compute measured weight from ADC read.
 *
 * @param iMeasuredWeight Pointer where weight will be stored. Must not be null.
 * @return true on success, false otherwise.
 */
bool bScale_getWeight(int * const iMeasuredWeight);

/**
 * @brief Set the current weight as reference. (Tare the scale)
 * Should be called some seconds after initialization.
 *
 * @return Nothing
 */
void vScale_tareScale();

/**
 * @brief Cleanup Scale I/O layer. Must be called before exiting.
 *
 * @return Nothing
 */
void vScale_cleanup();


#endif // _SCALE_H_
