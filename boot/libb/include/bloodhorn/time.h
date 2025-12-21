/*
 * time.h
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#ifndef BLOODHORN_TIME_H
#define BLOODHORN_TIME_H

#include <bloodhorn/types.h>
#include <bloodhorn/status.h>

#ifdef __cplusplus
extern "C" {
#endif

// Time representation
typedef struct {
    bh_uint16_t year;        // 1900-based year (e.g., 2023 = 123)
    bh_uint8_t  month;       // 1-12
    bh_uint8_t  day;         // 1-31
    bh_uint8_t  hour;        // 0-23
    bh_uint8_t  minute;      // 0-59
    bh_uint8_t  second;      // 0-59
    bh_uint32_t nanosecond;  // 0-999,999,999
    bh_int16_t  timezone;    // Minutes from UTC (-1440 to +1440)
    bh_uint8_t  daylight;    // Daylight saving time info
    bh_uint8_t  pad1;
} bh_time_t;

// Time capabilities
typedef struct {
    bh_uint32_t resolution;   // Timer resolution in nanoseconds
    bh_uint32_t accuracy;     // Timer accuracy in parts per million
    bh_bool_t sets_to_zero;   // Whether setting time resets the time to zero
} bh_time_capabilities_t;

// Clock types
typedef enum {
    BH_CLOCK_REALTIME = 0,    // System-wide real-time clock
    BH_CLOCK_MONOTONIC,       // Monotonic time since boot
    BH_CLOCK_PROCESS_CPUTIME, // High-resolution per-process timer
    BH_CLOCK_THREAD_CPUTIME,  // Thread-specific CPU-time clock
    BH_CLOCK_BOOTTIME,        // Boot time clock
    BH_CLOCK_TAI,             // International Atomic Time
    BH_CLOCK_MAX
} bh_clock_type_t;

// Timer types
typedef enum {
    BH_TIMER_ONESHOT = 0,     // One-shot timer
    BH_TIMER_PERIODIC         // Periodic timer
} bh_timer_type_t;

// Timer handle (opaque)
typedef struct bh_timer bh_timer_t;

// Timer callback function
typedef void (*bh_timer_callback_t)(bh_timer_t* timer, void* context);

// Timer specification
typedef struct {
    bh_timer_type_t type;
    bh_uint64_t interval_ns;  // Interval in nanoseconds
    bh_timer_callback_t callback;
    void* callback_context;
    bh_bool_t high_resolution;
} bh_timer_spec_t;

// Timezone information
typedef struct {
    char name[64];            // Timezone name (e.g., "UTC", "EST")
    char abbreviation[8];     // Timezone abbreviation (e.g., "UTC", "EST")
    bh_int32_t offset_seconds; // Offset from UTC in seconds
    bh_bool_t is_dst;         // Whether currently in daylight saving time
    bh_time_t dst_start;      // When DST starts
    bh_time_t dst_end;        // When DST ends
} bh_timezone_t;

// Performance counter frequency cache
extern bh_uint64_t bh_performance_frequency;

/**
 * @brief Initialize the time subsystem
 * 
 * @return bh_status_t Status code
 */
bh_status_t bh_time_initialize(void);

/**
 * @brief Shutdown the time subsystem
 */
void bh_time_shutdown(void);

/**
 * @brief Get the current time
 * 
 * @param time [out] Current time
 * @param capabilities [out] Time capabilities (can be NULL)
 * @return bh_status_t Status code
 */
bh_status_t bh_get_time(bh_time_t* time, bh_time_capabilities_t* capabilities);

/**
 * @brief Set the current time
 * 
 * @param time Time to set
 * @return bh_status_t Status code
 */
bh_status_t bh_set_time(const bh_time_t* time);

/**
 * @brief Get time from a specific clock
 * 
 * @param clock_type Type of clock to query
 * @param time [out] Current time
 * @return bh_status_t Status code
 */
bh_status_t bh_clock_gettime(bh_clock_type_t clock_type, bh_time_t* time);

/**
 * @brief Set time for a specific clock
 * 
 * @param clock_type Type of clock to set
 * @param time Time to set
 * @return bh_status_t Status code
 */
bh_status_t bh_clock_settime(bh_clock_type_t clock_type, const bh_time_t* time);

/**
 * @brief Get clock resolution
 * 
 * @param clock_type Type of clock
 * @param resolution [out] Clock resolution in nanoseconds
 * @return bh_status_t Status code
 */
bh_status_t bh_clock_getres(bh_clock_type_t clock_type, bh_uint64_t* resolution);

/**
 * @brief Get high-resolution timestamp
 * 
 * @return bh_uint64_t Timestamp in nanoseconds since boot
 */
bh_uint64_t bh_get_timestamp(void);

/**
 * @brief Get performance counter frequency
 * 
 * @return bh_uint64_t Frequency in Hz
 */
bh_uint64_t bh_get_performance_frequency(void);

/**
 * @brief Get performance counter value
 * 
 * @return bh_uint64_t Current counter value
 */
bh_uint64_t bh_get_performance_counter(void);

/**
 * @brief Convert performance counter ticks to nanoseconds
 * 
 * @param ticks Performance counter ticks
 * @return bh_uint64_t Nanoseconds
 */
bh_uint64_t bh_ticks_to_nanoseconds(bh_uint64_t ticks);

/**
 * @brief Convert nanoseconds to performance counter ticks
 * 
 * @param nanoseconds Nanoseconds
 * @return bh_uint64_t Performance counter ticks
 */
bh_uint64_t bh_nanoseconds_to_ticks(bh_uint64_t nanoseconds);

/**
 * @brief Sleep for a specified number of microseconds
 * 
 * @param microseconds Number of microseconds to sleep
 * @return bh_status_t Status code
 */
bh_status_t bh_sleep_microseconds(bh_uint64_t microseconds);

/**
 * @brief Sleep for a specified number of milliseconds
 * 
 * @param milliseconds Number of milliseconds to sleep
 * @return bh_status_t Status code
 */
bh_status_t bh_sleep_milliseconds(bh_uint64_t milliseconds);

/**
 * @brief Sleep for a specified number of seconds
 * 
 * @param seconds Number of seconds to sleep
 * @return bh_status_t Status code
 */
bh_status_t bh_sleep_seconds(bh_uint64_t seconds);

/**
 * @brief Busy-wait for a specified number of microseconds
 * 
 * @param microseconds Number of microseconds to wait
 */
void bh_busy_wait_microseconds(bh_uint64_t microseconds);

/**
 * @brief Create a timer
 * 
 * @param spec Timer specification
 * @param timer [out] Timer handle
 * @return bh_status_t Status code
 */
bh_status_t bh_timer_create(const bh_timer_spec_t* spec, bh_timer_t** timer);

/**
 * @brief Start a timer
 * 
 * @param timer Timer handle
 * @param delay_ns Initial delay in nanoseconds
 * @return bh_status_t Status code
 */
bh_status_t bh_timer_start(bh_timer_t* timer, bh_uint64_t delay_ns);

/**
 * @brief Stop a timer
 * 
 * @param timer Timer handle
 * @return bh_status_t Status code
 */
bh_status_t bh_timer_stop(bh_timer_t* timer);

/**
 * @brief Destroy a timer
 * 
 * @param timer Timer handle
 * @return bh_status_t Status code
 */
bh_status_t bh_timer_destroy(bh_timer_t* timer);

/**
 * @brief Check if a timer is active
 * 
 * @param timer Timer handle
 * @return bh_bool_t true if timer is active, false otherwise
 */
bh_bool_t bh_timer_is_active(bh_timer_t* timer);

/**
 * @brief Get remaining time for a timer
 * 
 * @param timer Timer handle
 * @param remaining_ns [out] Remaining time in nanoseconds
 * @return bh_status_t Status code
 */
bh_status_t bh_timer_get_remaining(bh_timer_t* timer, bh_uint64_t* remaining_ns);

/**
 * @brief Convert time to UNIX timestamp
 * 
 * @param time Time to convert
 * @return bh_uint64_t UNIX timestamp (seconds since epoch)
 */
bh_uint64_t bh_time_to_unix(const bh_time_t* time);

/**
 * @brief Convert UNIX timestamp to time
 * 
 * @param unix_time UNIX timestamp
 * @param time [out] Time structure
 * @return bh_status_t Status code
 */
bh_status_t bh_time_from_unix(bh_uint64_t unix_time, bh_time_t* time);

/**
 * @brief Add time duration to a time
 * 
 * @param time Base time
 * @param seconds Seconds to add
 * @param nanoseconds Nanoseconds to add
 * @param result [out] Result time
 * @return bh_status_t Status code
 */
bh_status_t bh_time_add(
    const bh_time_t* time,
    bh_int64_t seconds,
    bh_int64_t nanoseconds,
    bh_time_t* result
);

/**
 * @brief Subtract time duration from a time
 * 
 * @param time Base time
 * @param seconds Seconds to subtract
 * @param nanoseconds Nanoseconds to subtract
 * @param result [out] Result time
 * @return bh_status_t Status code
 */
bh_status_t bh_time_subtract(
    const bh_time_t* time,
    bh_int64_t seconds,
    bh_int64_t nanoseconds,
    bh_time_t* result
);

/**
 * @brief Compare two times
 * 
 * @param time1 First time
 * @param time2 Second time
 * @return bh_int32_t -1 if time1 < time2, 0 if equal, 1 if time1 > time2
 */
bh_int32_t bh_time_compare(const bh_time_t* time1, const bh_time_t* time2);

/**
 * @brief Get difference between two times
 * 
 * @param time1 First time
 * @param time2 Second time
 * @param diff_seconds [out] Difference in seconds
 * @param diff_nanoseconds [out] Difference in nanoseconds
 * @return bh_status_t Status code
 */
bh_status_t bh_time_diff(
    const bh_time_t* time1,
    const bh_time_t* time2,
    bh_int64_t* diff_seconds,
    bh_int64_t* diff_nanoseconds
);

/**
 * @brief Check if a year is a leap year
 * 
 * @param year Year to check
 * @return bh_bool_t true if leap year, false otherwise
 */
bh_bool_t bh_is_leap_year(bh_uint16_t year);

/**
 * @brief Get number of days in a month
 * 
 * @param year Year
 * @param month Month (1-12)
 * @return bh_uint8_t Number of days in the month
 */
bh_uint8_t bh_days_in_month(bh_uint16_t year, bh_uint8_t month);

/**
 * @brief Get day of the week
 * 
 * @param time Time to check
 * @return bh_uint8_t Day of week (0=Sunday, 1=Monday, ..., 6=Saturday)
 */
bh_uint8_t bh_day_of_week(const bh_time_t* time);

/**
 * @brief Get day of the year
 * 
 * @param time Time to check
 * @return bh_uint16_t Day of year (1-366)
 */
bh_uint16_t bh_day_of_year(const bh_time_t* time);

/**
 * @brief Format time as string
 * 
 * @param time Time to format
 * @param format Format string (strftime-style)
 * @param buffer Output buffer
 * @param size Buffer size
 * @return bh_size_t Number of characters written (0 on error)
 */
bh_size_t bh_time_format(
    const bh_time_t* time,
    const char* format,
    char* buffer,
    bh_size_t size
);

/**
 * @brief Parse time from string
 * 
 * @param string String to parse
 * @param format Format string (strptime-style)
 * @param time [out] Parsed time
 * @return bh_status_t Status code
 */
bh_status_t bh_time_parse(
    const char* string,
    const char* format,
    bh_time_t* time
);

/**
 * @brief Get timezone information
 * 
 * @param timezone [out] Timezone information
 * @return bh_status_t Status code
 */
bh_status_t bh_get_timezone(bh_timezone_t* timezone);

/**
 * @brief Set timezone
 * 
 * @param timezone Timezone information
 * @return bh_status_t Status code
 */
bh_status_t bh_set_timezone(const bh_timezone_t* timezone);

/**
 * @brief Convert time between timezones
 * 
 * @param time Input time
 * @param from_tz Source timezone
 * @param to_tz Destination timezone
 * @param result [out] Converted time
 * @return bh_status_t Status code
 */
bh_status_t bh_time_convert_timezone(
    const bh_time_t* time,
    const bh_timezone_t* from_tz,
    const bh_timezone_t* to_tz,
    bh_time_t* result
);

// Convenient time constants (in nanoseconds)
#define BH_TIME_NS_PER_MICROSECOND  1000ULL
#define BH_TIME_NS_PER_MILLISECOND  1000000ULL
#define BH_TIME_NS_PER_SECOND       1000000000ULL
#define BH_TIME_NS_PER_MINUTE       60000000000ULL
#define BH_TIME_NS_PER_HOUR         3600000000000ULL
#define BH_TIME_NS_PER_DAY          86400000000000ULL

// Convenient time macros
#define BH_MICROSECONDS(us) ((us) * BH_TIME_NS_PER_MICROSECOND)
#define BH_MILLISECONDS(ms) ((ms) * BH_TIME_NS_PER_MILLISECOND)
#define BH_SECONDS(s)       ((s) * BH_TIME_NS_PER_SECOND)
#define BH_MINUTES(m)       ((m) * BH_TIME_NS_PER_MINUTE)
#define BH_HOURS(h)         ((h) * BH_TIME_NS_PER_HOUR)
#define BH_DAYS(d)          ((d) * BH_TIME_NS_PER_DAY)

#ifdef __cplusplus
}
#endif

#endif // BLOODHORN_TIME_H
