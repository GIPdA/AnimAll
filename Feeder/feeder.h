/**
 * @file feeder.c
 *
 * @brief Feeder controller
 *
 * @author Benjamin Balga
 */

#ifndef _FEEDER_H_
#define _FEEDER_H_


/**
 * @brief Setup the Feeder I/O layer. Must be called first WITH ROOT RIGHTS !!
 *
 * @return true if feeder I/O layer successfully initialized, false otherwise.
 */
bool bFeeder_setup();

/**
 * @brief Start a Feeder cycle. Can be called multiple times, feed cycles will be queued.
 * @sa vFeeder_stop
 * @sa vFeeder_cancel
 *
 * @return The number of remaining cycles.
 */
unsigned int uiFeeder_startCycle();

/**
 * @brief Stop all Feeder cycles. Motor is stopped right away and the cycle count is reset.
 * @sa uiFeeder_startCycle
 * @sa vFeeder_cancel
 *
 * @return Nothing
 */
void vFeeder_stop();

/**
 * @brief Stop Feeder at end of current cycle. The cycle count is reset.
 * @sa vFeeder_stop
 * @sa uiFeeder_startCycle
 *
 * @return Nothing
 */
void vFeeder_cancel();

/**
 * @brief Get the running state of the feeder
 *
 * @return Running state
 */
bool bFeeder_isRunning();

/**
 * @brief Cleanup Feeder I/O layer. Must be called before exiting.
 *
 * @return Nothing
 */
void vFeeder_cleanup();


#endif // _FEEDER_H_
