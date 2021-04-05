#ifndef VERSION_H_
#define VERSION_H_

#define MON_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define MON_IS_FEB (__DATE__[0] == 'F' && __DATE__[1] == 'e' && __DATE__[2] == 'b')
#define MON_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define MON_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p' && __DATE__[2] == 'r')
#define MON_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define MON_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define MON_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define MON_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u' && __DATE__[2] == 'g')
#define MON_IS_SEP (__DATE__[0] == 'S' && __DATE__[1] == 'e' && __DATE__[2] == 'p')
#define MON_IS_OCT (__DATE__[0] == 'O' && __DATE__[1] == 'c' && __DATE__[2] == 't')
#define MON_IS_NOV (__DATE__[0] == 'N' && __DATE__[1] == 'o' && __DATE__[2] == 'v')
#define MON_IS_DEC (__DATE__[0] == 'D' && __DATE__[1] == 'e' && __DATE__[2] == 'c')

//=============================================================================

// Year
#define YEAR_CH0 (__DATE__[7])
#define YEAR_CH1 (__DATE__[8])
#define YEAR_CH2 (__DATE__[9])
#define YEAR_CH3 (__DATE__[10])
// MON
#define MON_CH0           \
    ((MON_IS_JAN)   ? '0' \
     : (MON_IS_FEB) ? '0' \
     : (MON_IS_MAR) ? '0' \
     : (MON_IS_APR) ? '0' \
     : (MON_IS_MAY) ? '0' \
     : (MON_IS_JUN) ? '0' \
     : (MON_IS_JUL) ? '0' \
     : (MON_IS_AUG) ? '0' \
     : (MON_IS_SEP) ? '0' \
     : (MON_IS_OCT) ? '1' \
     : (MON_IS_NOV) ? '1' \
     : (MON_IS_DEC) ? '1' \
                    : ' ')
#define MON_CH1           \
    ((MON_IS_JAN)   ? '1' \
     : (MON_IS_FEB) ? '2' \
     : (MON_IS_MAR) ? '3' \
     : (MON_IS_APR) ? '4' \
     : (MON_IS_MAY) ? '5' \
     : (MON_IS_JUN) ? '6' \
     : (MON_IS_JUL) ? '7' \
     : (MON_IS_AUG) ? '8' \
     : (MON_IS_SEP) ? '9' \
     : (MON_IS_OCT) ? '0' \
     : (MON_IS_NOV) ? '1' \
     : (MON_IS_DEC) ? '2' \
                    : ' ')
// Date
#define DAY_CH0 ((__DATE__[4]) == ' ' ? '0' : (__DATE__[4]))
#define DAY_CH1 ((__DATE__[5]) == ' ' ? '0' : (__DATE__[5]))

//=============================================================================

// Hour
#define HH_CH0 (__TIME__[0])
#define HH_CH1 (__TIME__[1])
// Minute
#define MM_CH0 (__TIME__[3])
#define MM_CH1 (__TIME__[4])
// Second
#define SS_CH0 (__TIME__[6])
#define SS_CH1 (__TIME__[7])

//=============================================================================

const char version[] =
    {'F', 'L', 'O',
     '-',
     YEAR_CH0, YEAR_CH1, YEAR_CH2, YEAR_CH3,
     MON_CH0, MON_CH1,
     DAY_CH0, DAY_CH1,
     '-',
     HH_CH0, HH_CH1,
     MM_CH0, MM_CH1,
     SS_CH0, SS_CH1,
     '\0'};

#endif
