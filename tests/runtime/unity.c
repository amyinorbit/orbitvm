/* =========================================================================
    Unity Project - A Test Framework for C
    Copyright (c) 2007-14 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
============================================================================ */

#include "unity.h"
#include <stddef.h>

/* If omitted from header, declare overrideable prototypes here so they're ready for use */
#ifdef UNITY_OMIT_OUTPUT_CHAR_HEADER_DECLARATION
void UNITY_OUTPUT_CHAR(int);
#endif

/* Helpful macros for us to use here */
#define UNITY_FAIL_AND_BAIL   { Unity.CurrentTestFailed  = 1; longjmp(Unity.AbortFrame, 1); }
#define UNITY_IGNORE_AND_BAIL { Unity.CurrentTestIgnored = 1; longjmp(Unity.AbortFrame, 1); }

/* return prematurely if we are already in failure or ignore state */
#define UNITY_SKIP_EXECUTION  { if ((Unity.CurrentTestFailed != 0) || (Unity.CurrentTestIgnored != 0)) {return;} }

struct UNITY_STORAGE_T Unity;

static const char UnityStrOk[]                     = "OK";
static const char UnityStrPass[]                   = "PASS";
static const char UnityStrFail[]                   = "FAIL";
static const char UnityStrIgnore[]                 = "IGNORE";
static const char UnityStrNull[]                   = "NULL";
static const char UnityStrSpacer[]                 = ". ";
static const char UnityStrExpected[]               = " Expected ";
static const char UnityStrWas[]                    = " Was ";
static const char UnityStrElement[]                = " Element ";
static const char UnityStrByte[]                   = " Byte ";
static const char UnityStrMemory[]                 = " Memory Mismatch.";
static const char UnityStrDelta[]                  = " Values Not Within Delta ";
static const char UnityStrPointless[]              = " You Asked Me To Compare Nothing, Which Was Pointless.";
static const char UnityStrNullPointerForExpected[] = " Expected pointer to be NULL";
static const char UnityStrNullPointerForActual[]   = " Actual pointer was NULL";
#ifndef UNITY_EXCLUDE_FLOAT
static const char UnityStrNot[]                    = "Not ";
static const char UnityStrInf[]                    = "Infinity";
static const char UnityStrNegInf[]                 = "Negative Infinity";
static const char UnityStrNaN[]                    = "NaN";
static const char UnityStrDet[]                    = "Determinate";
static const char UnityStrInvalidFloatTrait[]      = "Invalid Float Trait";
#endif
const char UnityStrErrFloat[]                      = "Unity Floating Point Disabled";
const char UnityStrErrDouble[]                     = "Unity Double Precision Disabled";
const char UnityStrErr64[]                         = "Unity 64-bit Support Disabled";
static const char UnityStrBreaker[]                = "-----------------------";
static const char UnityStrResultsTests[]           = " Tests ";
static const char UnityStrResultsFailures[]        = " Failures ";
static const char UnityStrResultsIgnored[]         = " Ignored ";
static const char UnityStrDetail1Name[]            = UNITY_DETAIL1_NAME " ";
static const char UnityStrDetail2Name[]            = " " UNITY_DETAIL2_NAME " ";

/* compiler-generic print formatting masks */
static const UNITY_UINT UnitySizeMask[] =
{
    255u,         /* 0xFF */
    65535u,       /* 0xFFFF */
    65535u,
    4294967295u,  /* 0xFFFFFFFF */
    4294967295u,
    4294967295u,
    4294967295u
#ifdef UNITY_SUPPORT_64
    ,0xFFFFFFFFFFFFFFFF
#endif
};

/*-----------------------------------------------
 * Pretty Printers & Test Result Output Handlers
 *-----------------------------------------------*/

void UnityPrint(const char* string)
{
    const char* pch = string;

    if (pch != NULL)
    {
        while (*pch)
        {
            /* printable characters plus CR & LF are printed */
            if ((*pch <= 126) && (*pch >= 32))
            {
                UNITY_OUTPUT_CHAR(*pch);
            }
            /* write escaped carriage returns */
            else if (*pch == 13)
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('r');
            }
            /* write escaped line feeds */
            else if (*pch == 10)
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('n');
            }
            /* unprintable characters are shown as codes */
            else
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('x');
                UnityPrintNumberHex((UNITY_UINT)*pch, 2);
            }
            pch++;
        }
    }
}

void UnityPrintLen(const char* string, const UNITY_UINT32 length);
void UnityPrintLen(const char* string, const UNITY_UINT32 length)
{
    const char* pch = string;

    if (pch != NULL)
    {
        while (*pch && (UNITY_UINT32)(pch - string) < length)
        {
            /* printable characters plus CR & LF are printed */
            if ((*pch <= 126) && (*pch >= 32))
            {
                UNITY_OUTPUT_CHAR(*pch);
            }
            /* write escaped carriage returns */
            else if (*pch == 13)
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('r');
            }
            /* write escaped line feeds */
            else if (*pch == 10)
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('n');
            }
            /* unprintable characters are shown as codes */
            else
            {
                UNITY_OUTPUT_CHAR('\\');
                UNITY_OUTPUT_CHAR('x');
                UnityPrintNumberHex((UNITY_UINT)*pch, 2);
            }
            pch++;
        }
    }
}

/*-----------------------------------------------*/
void UnityPrintNumberByStyle(const UNITY_INT number, const UNITY_DISPLAY_STYLE_T style)
{
    if ((style & UNITY_DISPLAY_RANGE_INT) == UNITY_DISPLAY_RANGE_INT)
    {
        UnityPrintNumber(number);
    }
    else if ((style & UNITY_DISPLAY_RANGE_UINT) == UNITY_DISPLAY_RANGE_UINT)
    {
        UnityPrintNumberUnsigned(  (UNITY_UINT)number  &  UnitySizeMask[((UNITY_UINT)style & (UNITY_UINT)0x0F) - 1]  );
    }
    else
    {
        UNITY_OUTPUT_CHAR('0');
        UNITY_OUTPUT_CHAR('x');
        UnityPrintNumberHex((UNITY_UINT)number, (char)((style & 0x000F) << 1));
    }
}

/*-----------------------------------------------*/
void UnityPrintNumber(const UNITY_INT number_to_print)
{
    UNITY_UINT number = (UNITY_UINT)number_to_print;

    if (number_to_print < 0)
    {
        /* A negative number, including MIN negative */
        UNITY_OUTPUT_CHAR('-');
        number = (UNITY_UINT)(-number_to_print);
    }
    UnityPrintNumberUnsigned(number);
}

/*-----------------------------------------------
 * basically do an itoa using as little ram as possible */
void UnityPrintNumberUnsigned(const UNITY_UINT number)
{
    UNITY_UINT divisor = 1;

    /* figure out initial divisor */
    while (number / divisor > 9)
    {
        divisor *= 10;
    }

    /* now mod and print, then divide divisor */
    do
    {
        UNITY_OUTPUT_CHAR((char)('0' + (number / divisor % 10)));
        divisor /= 10;
    }
    while (divisor > 0);
}

/*-----------------------------------------------*/
void UnityPrintNumberHex(const UNITY_UINT number, const char nibbles_to_print)
{
    UNITY_UINT nibble;
    char nibbles = nibbles_to_print;

    while (nibbles > 0)
    {
        nibble = (number >> (--nibbles << 2)) & 0x0000000F;
        if (nibble <= 9)
        {
            UNITY_OUTPUT_CHAR((char)('0' + nibble));
        }
        else
        {
            UNITY_OUTPUT_CHAR((char)('A' - 10 + nibble));
        }
    }
}

/*-----------------------------------------------*/
void UnityPrintMask(const UNITY_UINT mask, const UNITY_UINT number)
{
    UNITY_UINT current_bit = (UNITY_UINT)1 << (UNITY_INT_WIDTH - 1);
    UNITY_INT32 i;

    for (i = 0; i < UNITY_INT_WIDTH; i++)
    {
        if (current_bit & mask)
        {
            if (current_bit & number)
            {
                UNITY_OUTPUT_CHAR('1');
            }
            else
            {
                UNITY_OUTPUT_CHAR('0');
            }
        }
        else
        {
            UNITY_OUTPUT_CHAR('X');
        }
        current_bit = current_bit >> 1;
    }
}

/*-----------------------------------------------*/
#ifndef UNITY_EXCLUDE_FLOAT_PRINT
static void UnityPrintDecimalAndNumberWithLeadingZeros(UNITY_INT32 fraction_part, UNITY_INT32 divisor)
{
    UNITY_OUTPUT_CHAR('.');
    while (divisor > 0)
    {
        UNITY_OUTPUT_CHAR('0' + fraction_part / divisor);
        fraction_part %= divisor;
        divisor /= 10;
        if (fraction_part == 0) break; /* Truncate trailing 0's */
    }
}
#ifndef UNITY_ROUND_TIES_AWAY_FROM_ZERO
/* If rounds up && remainder 0.5 && result odd && below cutoff for double precision issues */
  #define ROUND_TIES_TO_EVEN(orig, num_int, num)                                          \
  if (num_int > (num) && (num) - (num_int-1) <= 0.5 && (num_int & 1) == 1 && orig < 1e22) \
    num_int -= 1 /* => a tie to round down to even */
#else
  #define ROUND_TIES_TO_EVEN(orig, num_int, num) /* Remove macro */
#endif

/* Printing floating point numbers is hard. Some goals of this implementation: works for embedded
 * systems, floats or doubles, and has a reasonable format. The key paper in this area,
 * 'How to Print Floating-Point Numbers Accurately' by Steele & White, shows an approximation by
 * scaling called Dragon 2. This code uses a similar idea. The other core algorithm uses casts and
 * floating subtraction to give exact remainders after the decimal, to be scaled into an integer.
 * Extra trailing 0's are excluded. The output defaults to rounding to nearest, ties to even. You
 * can enable rounding ties away from zero. Note: UNITY_DOUBLE param can typedef to float or double

 * The old version required compiling in snprintf. For reference, with a similar format as now:
 *  char buf[19];
 *  if (number > 4294967296.0 || -number > 4294967296.0) snprintf(buf, sizeof buf, "%.8e", number);
 *  else                                                 snprintf(buf, sizeof buf, "%.6f", number);
 *  UnityPrint(buf);
 */
void UnityPrintFloat(UNITY_DOUBLE number)
{
    if (number < 0)
    {
        UNITY_OUTPUT_CHAR('-');
        number = -number;
    }

    if (isnan(number)) UnityPrint(UnityStrNaN);
    else if (isinf(number)) UnityPrintLen(UnityStrInf, 3);
    else if (number <= 0.0000005 && number > 0) UnityPrint("0.000000..."); /* Small number */
    else if (number < 4294967295.9999995) /* Rounded result fits in 32 bits, "%.6f" format */
    {
        const UNITY_INT32 divisor = 1000000/10;
        UNITY_UINT32 integer_part = (UNITY_UINT32)number;
        UNITY_INT32 fraction_part = (UNITY_INT32)((number - (UNITY_DOUBLE)integer_part)*1000000.0 + 0.5);
        /* Double precision calculation gives best performance for six rounded decimal places */
        ROUND_TIES_TO_EVEN(number, fraction_part, (number - (UNITY_DOUBLE)integer_part)*1000000.0);

        if (fraction_part == 1000000) /* Carry across the decimal point */
        {
            fraction_part = 0;
            integer_part += 1;
        }

        UnityPrintNumberUnsigned(integer_part);
        UnityPrintDecimalAndNumberWithLeadingZeros(fraction_part, divisor);
    }
    else /* Number is larger, use exponential format of 9 digits, "%.8e" */
    {
        const UNITY_INT32 divisor = 1000000000/10;
        UNITY_INT32 integer_part;
        UNITY_DOUBLE_TYPE divide = 10.0;
        int exponent = 9;

        while (number / divide >= 1000000000.0 - 0.5)
        {
            divide *= 10;
            exponent++;
        }
        integer_part = (UNITY_INT32)(number / divide + 0.5);
        /* Double precision calculation required for float, to produce 9 rounded digits */
        ROUND_TIES_TO_EVEN(number, integer_part, number / divide);

        UNITY_OUTPUT_CHAR('0' + integer_part / divisor);
        UnityPrintDecimalAndNumberWithLeadingZeros(integer_part % divisor, divisor / 10);
        UNITY_OUTPUT_CHAR('e');
        UNITY_OUTPUT_CHAR('+');
        if (exponent < 10) UNITY_OUTPUT_CHAR('0');
        UnityPrintNumber(exponent);
    }
}
#endif /* ! UNITY_EXCLUDE_FLOAT_PRINT */

/*-----------------------------------------------*/

void UnityPrintFail(void);
void UnityPrintFail(void)
{
    UnityPrint(UnityStrFail);
}

void UnityPrintOk(void);
void UnityPrintOk(void)
{
    UnityPrint(UnityStrOk);
}

/*-----------------------------------------------*/
static void UnityTestResultsBegin(const char* file, const UNITY_LINE_TYPE line);
static void UnityTestResultsBegin(const char* file, const UNITY_LINE_TYPE line)
{
#ifndef UNITY_FIXTURES
    UnityPrint(file);
    UNITY_OUTPUT_CHAR(':');
    UnityPrintNumber((UNITY_INT)line);
    UNITY_OUTPUT_CHAR(':');
    UnityPrint(Unity.CurrentTestName);
    UNITY_OUTPUT_CHAR(':');
#else
    UNITY_UNUSED(file);
    UNITY_UNUSED(line);
#endif
}

/*-----------------------------------------------*/
static void UnityTestResultsFailBegin(const UNITY_LINE_TYPE line);
static void UnityTestResultsFailBegin(const UNITY_LINE_TYPE line)
{
#ifndef UNITY_FIXTURES
    UnityTestResultsBegin(Unity.TestFile, line);
#else
    UNITY_UNUSED(line);
#endif
    UnityPrint(UnityStrFail);
    UNITY_OUTPUT_CHAR(':');
}

/*-----------------------------------------------*/
void UnityConcludeTest(void)
{
    if (Unity.CurrentTestIgnored)
    {
        Unity.TestIgnores++;
    }
    else if (!Unity.CurrentTestFailed)
    {
        UnityTestResultsBegin(Unity.TestFile, Unity.CurrentTestLineNumber);
        UnityPrint(UnityStrPass);
    }
    else
    {
        Unity.TestFailures++;
    }

    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;
    UNITY_PRINT_EOL();
    UNITY_FLUSH_CALL();
}

/*-----------------------------------------------*/
static void UnityAddMsgIfSpecified(const char* msg);
static void UnityAddMsgIfSpecified(const char* msg)
{
    if (msg)
    {
        UnityPrint(UnityStrSpacer);
#ifndef UNITY_EXCLUDE_DETAILS
        if (Unity.CurrentDetail1)
        {
            UnityPrint(UnityStrDetail1Name);
            UnityPrint(Unity.CurrentDetail1);
            if (Unity.CurrentDetail2)
            {
                UnityPrint(UnityStrDetail2Name);
                UnityPrint(Unity.CurrentDetail2);
            }
            UnityPrint(UnityStrSpacer);
        }
#endif
        UnityPrint(msg);
    }
}

/*-----------------------------------------------*/
static void UnityPrintExpectedAndActualStrings(const char* expected, const char* actual);
static void UnityPrintExpectedAndActualStrings(const char* expected, const char* actual)
{
    UnityPrint(UnityStrExpected);
    if (expected != NULL)
    {
        UNITY_OUTPUT_CHAR('\'');
        UnityPrint(expected);
        UNITY_OUTPUT_CHAR('\'');
    }
    else
    {
      UnityPrint(UnityStrNull);
    }
    UnityPrint(UnityStrWas);
    if (actual != NULL)
    {
        UNITY_OUTPUT_CHAR('\'');
        UnityPrint(actual);
        UNITY_OUTPUT_CHAR('\'');
    }
    else
    {
      UnityPrint(UnityStrNull);
    }
}

/*-----------------------------------------------*/
static void UnityPrintExpectedAndActualStringsLen(const char* expected, const char* actual, const UNITY_UINT32 length)
{
    UnityPrint(UnityStrExpected);
    if (expected != NULL)
    {
        UNITY_OUTPUT_CHAR('\'');
        UnityPrintLen(expected, length);
        UNITY_OUTPUT_CHAR('\'');
    }
    else
    {
      UnityPrint(UnityStrNull);
    }
    UnityPrint(UnityStrWas);
    if (actual != NULL)
    {
        UNITY_OUTPUT_CHAR('\'');
        UnityPrintLen(actual, length);
        UNITY_OUTPUT_CHAR('\'');
    }
    else
    {
      UnityPrint(UnityStrNull);
    }
}



/*-----------------------------------------------
 * Assertion & Control Helpers
 *-----------------------------------------------*/

static int UnityCheckArraysForNull(UNITY_INTERNAL_PTR expected, UNITY_INTERNAL_PTR actual, const UNITY_LINE_TYPE lineNumber, const char* msg)
{
    /* return true if they are both NULL */
    if ((expected == NULL) && (actual == NULL))
        return 1;

    /* throw error if just expected is NULL */
    if (expected == NULL)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrNullPointerForExpected);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }

    /* throw error if just actual is NULL */
    if (actual == NULL)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrNullPointerForActual);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }

    /* return false if neither is NULL */
    return 0;
}

/*-----------------------------------------------
 * Assertion Functions
 *-----------------------------------------------*/

void UnityAssertBits(const UNITY_INT mask,
                     const UNITY_INT expected,
                     const UNITY_INT actual,
                     const char* msg,
                     const UNITY_LINE_TYPE lineNumber)
{
    UNITY_SKIP_EXECUTION;

    if ((mask & expected) != (mask & actual))
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrExpected);
        UnityPrintMask((UNITY_UINT)mask, (UNITY_UINT)expected);
        UnityPrint(UnityStrWas);
        UnityPrintMask((UNITY_UINT)mask, (UNITY_UINT)actual);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/
void UnityAssertEqualNumber(const UNITY_INT expected,
                            const UNITY_INT actual,
                            const char* msg,
                            const UNITY_LINE_TYPE lineNumber,
                            const UNITY_DISPLAY_STYLE_T style)
{
    UNITY_SKIP_EXECUTION;

    if (expected != actual)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrExpected);
        UnityPrintNumberByStyle(expected, style);
        UnityPrint(UnityStrWas);
        UnityPrintNumberByStyle(actual, style);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

#define UnityPrintPointlessAndBail()       \
{                                          \
    UnityTestResultsFailBegin(lineNumber); \
    UnityPrint(UnityStrPointless);         \
    UnityAddMsgIfSpecified(msg);           \
    UNITY_FAIL_AND_BAIL; }

/*-----------------------------------------------*/
void UnityAssertEqualIntArray(UNITY_INTERNAL_PTR expected,
                              UNITY_INTERNAL_PTR actual,
                              const UNITY_UINT32 num_elements,
                              const char* msg,
                              const UNITY_LINE_TYPE lineNumber,
                              const UNITY_DISPLAY_STYLE_T style)
{
    UNITY_UINT32 elements = num_elements;
    UNITY_INTERNAL_PTR ptr_exp = (UNITY_INTERNAL_PTR)expected;
    UNITY_INTERNAL_PTR ptr_act = (UNITY_INTERNAL_PTR)actual;

    UNITY_SKIP_EXECUTION;

    if (elements == 0)
    {
        UnityPrintPointlessAndBail();
    }

    if (UnityCheckArraysForNull((UNITY_INTERNAL_PTR)expected, (UNITY_INTERNAL_PTR)actual, lineNumber, msg) == 1)
        return;

    /* If style is UNITY_DISPLAY_STYLE_INT, we'll fall into the default case rather than the INT16 or INT32 (etc) case
     * as UNITY_DISPLAY_STYLE_INT includes a flag for UNITY_DISPLAY_RANGE_AUTO, which the width-specific
     * variants do not. Therefore remove this flag. */
    switch(style & (UNITY_DISPLAY_STYLE_T)(~UNITY_DISPLAY_RANGE_AUTO))
    {
        case UNITY_DISPLAY_STYLE_HEX8:
        case UNITY_DISPLAY_STYLE_INT8:
        case UNITY_DISPLAY_STYLE_UINT8:
            while (elements--)
            {
                if (*(UNITY_PTR_ATTRIBUTE const UNITY_INT8*)ptr_exp != *(UNITY_PTR_ATTRIBUTE const UNITY_INT8*)ptr_act)
                {
                    UnityTestResultsFailBegin(lineNumber);
                    UnityPrint(UnityStrElement);
                    UnityPrintNumberUnsigned(num_elements - elements - 1);
                    UnityPrint(UnityStrExpected);
                    UnityPrintNumberByStyle(*(UNITY_PTR_ATTRIBUTE const UNITY_INT8*)ptr_exp, style);
                    UnityPrint(UnityStrWas);
                    UnityPrintNumberByStyle(*(UNITY_PTR_ATTRIBUTE const UNITY_INT8*)ptr_act, style);
                    UnityAddMsgIfSpecified(msg);
                    UNITY_FAIL_AND_BAIL;
                }
                ptr_exp = (UNITY_INTERNAL_PTR)((UNITY_PTR)ptr_exp + 1);
                ptr_act = (UNITY_INTERNAL_PTR)((UNITY_PTR)ptr_act + 1);
            }
            break;
        case UNITY_DISPLAY_STYLE_HEX16:
        case UNITY_DISPLAY_STYLE_INT16:
        case UNITY_DISPLAY_STYLE_UINT16:
            while (elements--)
            {
                if (*(UNITY_PTR_ATTRIBUTE const UNITY_INT16*)ptr_exp != *(UNITY_PTR_ATTRIBUTE const UNITY_INT16*)ptr_act)
                {
                    UnityTestResultsFailBegin(lineNumber);
                    UnityPrint(UnityStrElement);
                    UnityPrintNumberUnsigned(num_elements - elements - 1);
                    UnityPrint(UnityStrExpected);
                    UnityPrintNumberByStyle(*(UNITY_PTR_ATTRIBUTE const UNITY_INT16*)ptr_exp, style);
                    UnityPrint(UnityStrWas);
                    UnityPrintNumberByStyle(*(UNITY_PTR_ATTRIBUTE const UNITY_INT16*)ptr_act, style);
                    UnityAddMsgIfSpecified(msg);
                    UNITY_FAIL_AND_BAIL;
                }
                ptr_exp = (UNITY_INTERNAL_PTR)((UNITY_PTR)ptr_exp + 2);
                ptr_act = (UNITY_INTERNAL_PTR)((UNITY_PTR)ptr_act + 2);
            }
            break;
#ifdef UNITY_SUPPORT_64
        case UNITY_DISPLAY_STYLE_HEX64:
        case UNITY_DISPLAY_STYLE_INT64:
        case UNITY_DISPLAY_STYLE_UINT64:
            while (elements--)
            {
                if (*(UNITY_PTR_ATTRIBUTE const UNITY_INT64*)ptr_exp != *(UNITY_PTR_ATTRIBUTE const UNITY_INT64*)ptr_act)
                {
                    UnityTestResultsFailBegin(lineNumber);
                    UnityPrint(UnityStrElement);
                    UnityPrintNumberUnsigned(num_elements - elements - 1);
                    UnityPrint(UnityStrExpected);
                    UnityPrintNumberByStyle(*(UNITY_PTR_ATTRIBUTE const UNITY_INT64*)ptr_exp, style);
                    UnityPrint(UnityStrWas);
                    UnityPrintNumberByStyle(*(UNITY_PTR_ATTRIBUTE const UNITY_INT64*)ptr_act, style);
                    UnityAddMsgIfSpecified(msg);
                    UNITY_FAIL_AND_BAIL;
                }
                ptr_exp = (UNITY_INTERNAL_PTR)((UNITY_PTR)ptr_exp + 8);
                ptr_act = (UNITY_INTERNAL_PTR)((UNITY_PTR)ptr_act + 8);
            }
            break;
#endif
        default:
            while (elements--)
            {
                if (*(UNITY_PTR_ATTRIBUTE const UNITY_INT32*)ptr_exp != *(UNITY_PTR_ATTRIBUTE const UNITY_INT32*)ptr_act)
                {
                    UnityTestResultsFailBegin(lineNumber);
                    UnityPrint(UnityStrElement);
                    UnityPrintNumberUnsigned(num_elements - elements - 1);
                    UnityPrint(UnityStrExpected);
                    UnityPrintNumberByStyle(*(UNITY_PTR_ATTRIBUTE const UNITY_INT32*)ptr_exp, style);
                    UnityPrint(UnityStrWas);
                    UnityPrintNumberByStyle(*(UNITY_PTR_ATTRIBUTE const UNITY_INT32*)ptr_act, style);
                    UnityAddMsgIfSpecified(msg);
                    UNITY_FAIL_AND_BAIL;
                }
                ptr_exp = (UNITY_INTERNAL_PTR)((UNITY_PTR)ptr_exp + 4);
                ptr_act = (UNITY_INTERNAL_PTR)((UNITY_PTR)ptr_act + 4);
            }
            break;
    }
}

/*-----------------------------------------------*/
/* Wrap this define in a function with variable types as float or double */
#define UNITY_FLOAT_OR_DOUBLE_WITHIN(delta, expected, actual, diff)                       \
    if (isinf(expected) && isinf(actual) && (isneg(expected) == isneg(actual))) return 1; \
    if (UNITY_NAN_CHECK) return 1;                                                        \
    diff = actual - expected;                                                             \
    if (diff < 0.0f) diff = 0.0f - diff;                                                  \
    if (delta < 0.0f) delta = 0.0f - delta;                                               \
    return !(isnan(diff) || isinf(diff) || (diff > delta))
    /* This first part of this condition will catch any NaN or Infinite values */
#ifndef UNITY_NAN_NOT_EQUAL_NAN
  #define UNITY_NAN_CHECK isnan(expected) && isnan(actual)
#else
  #define UNITY_NAN_CHECK 0
#endif

#ifndef UNITY_EXCLUDE_FLOAT_PRINT
  #define UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT(expected, actual) \
    do {                                                          \
    UnityPrint(UnityStrExpected);                                 \
    UnityPrintFloat(expected);                                    \
    UnityPrint(UnityStrWas);                                      \
    UnityPrintFloat(actual);                                      \
    } while(0)
#else
  #define UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT(expected, actual) \
    UnityPrint(UnityStrDelta)
#endif /* UNITY_EXCLUDE_FLOAT_PRINT */

#ifndef UNITY_EXCLUDE_FLOAT
static int UnityFloatsWithin(UNITY_FLOAT delta, UNITY_FLOAT expected, UNITY_FLOAT actual)
{
    UNITY_FLOAT diff;
    UNITY_FLOAT_OR_DOUBLE_WITHIN(delta, expected, actual, diff);
}

void UnityAssertEqualFloatArray(UNITY_PTR_ATTRIBUTE const UNITY_FLOAT* expected,
                                UNITY_PTR_ATTRIBUTE const UNITY_FLOAT* actual,
                                const UNITY_UINT32 num_elements,
                                const char* msg,
                                const UNITY_LINE_TYPE lineNumber)
{
    UNITY_UINT32 elements = num_elements;
    UNITY_PTR_ATTRIBUTE const UNITY_FLOAT* ptr_expected = expected;
    UNITY_PTR_ATTRIBUTE const UNITY_FLOAT* ptr_actual = actual;

    UNITY_SKIP_EXECUTION;

    if (elements == 0)
    {
        UnityPrintPointlessAndBail();
    }

    if (UnityCheckArraysForNull((UNITY_INTERNAL_PTR)expected, (UNITY_INTERNAL_PTR)actual, lineNumber, msg) == 1)
        return;

    while (elements--)
    {
        if (!UnityFloatsWithin(*ptr_expected * UNITY_FLOAT_PRECISION, *ptr_expected, *ptr_actual))
        {
            UnityTestResultsFailBegin(lineNumber);
            UnityPrint(UnityStrElement);
            UnityPrintNumberUnsigned(num_elements - elements - 1);
            UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT(*ptr_expected, *ptr_actual);
            UnityAddMsgIfSpecified(msg);
            UNITY_FAIL_AND_BAIL;
        }
        ptr_expected++;
        ptr_actual++;
    }
}

/*-----------------------------------------------*/
void UnityAssertFloatsWithin(const UNITY_FLOAT delta,
                             const UNITY_FLOAT expected,
                             const UNITY_FLOAT actual,
                             const char* msg,
                             const UNITY_LINE_TYPE lineNumber)
{
    UNITY_SKIP_EXECUTION;


    if (!UnityFloatsWithin(delta, expected, actual))
    {
        UnityTestResultsFailBegin(lineNumber);
        UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT(expected, actual);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/
void UnityAssertFloatSpecial(const UNITY_FLOAT actual,
                             const char* msg,
                             const UNITY_LINE_TYPE lineNumber,
                             const UNITY_FLOAT_TRAIT_T style)
{
    const char* trait_names[] = { UnityStrInf, UnityStrNegInf, UnityStrNaN, UnityStrDet };
    UNITY_INT should_be_trait   = ((UNITY_INT)style & 1);
    UNITY_INT is_trait          = !should_be_trait;
    UNITY_INT trait_index       = (UNITY_INT)(style >> 1);

    UNITY_SKIP_EXECUTION;

    switch(style)
    {
        case UNITY_FLOAT_IS_INF:
        case UNITY_FLOAT_IS_NOT_INF:
            is_trait = isinf(actual) & ispos(actual);
            break;
        case UNITY_FLOAT_IS_NEG_INF:
        case UNITY_FLOAT_IS_NOT_NEG_INF:
            is_trait = isinf(actual) & isneg(actual);
            break;

        case UNITY_FLOAT_IS_NAN:
        case UNITY_FLOAT_IS_NOT_NAN:
            is_trait = isnan(actual) ? 1 : 0;
            break;

        /* A determinate number is non infinite and not NaN. (therefore the opposite of the two above) */
        case UNITY_FLOAT_IS_DET:
        case UNITY_FLOAT_IS_NOT_DET:
            if (isinf(actual) || isnan(actual))
                is_trait = 0;
            else
                is_trait = 1;
            break;

        default:
            trait_index = 0;
            trait_names[0] = UnityStrInvalidFloatTrait;
            break;
    }

    if (is_trait != should_be_trait)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrExpected);
        if (!should_be_trait)
            UnityPrint(UnityStrNot);
        UnityPrint(trait_names[trait_index]);
        UnityPrint(UnityStrWas);
#ifndef UNITY_EXCLUDE_FLOAT_PRINT
        UnityPrintFloat(actual);
#else
        if (should_be_trait)
            UnityPrint(UnityStrNot);
        UnityPrint(trait_names[trait_index]);
#endif
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

#endif /* not UNITY_EXCLUDE_FLOAT */

/*-----------------------------------------------*/
#ifndef UNITY_EXCLUDE_DOUBLE
static int UnityDoublesWithin(UNITY_DOUBLE delta, UNITY_DOUBLE expected, UNITY_DOUBLE actual)
{
    UNITY_DOUBLE diff;
    UNITY_FLOAT_OR_DOUBLE_WITHIN(delta, expected, actual, diff);
}

void UnityAssertEqualDoubleArray(UNITY_PTR_ATTRIBUTE const UNITY_DOUBLE* expected,
                                 UNITY_PTR_ATTRIBUTE const UNITY_DOUBLE* actual,
                                 const UNITY_UINT32 num_elements,
                                 const char* msg,
                                 const UNITY_LINE_TYPE lineNumber)
{
    UNITY_UINT32 elements = num_elements;
    UNITY_PTR_ATTRIBUTE const UNITY_DOUBLE* ptr_expected = expected;
    UNITY_PTR_ATTRIBUTE const UNITY_DOUBLE* ptr_actual = actual;

    UNITY_SKIP_EXECUTION;

    if (elements == 0)
    {
        UnityPrintPointlessAndBail();
    }

    if (UnityCheckArraysForNull((UNITY_INTERNAL_PTR)expected, (UNITY_INTERNAL_PTR)actual, lineNumber, msg) == 1)
        return;

    while (elements--)
    {
        if (!UnityDoublesWithin(*ptr_expected * UNITY_DOUBLE_PRECISION, *ptr_expected, *ptr_actual))
        {
            UnityTestResultsFailBegin(lineNumber);
            UnityPrint(UnityStrElement);
            UnityPrintNumberUnsigned(num_elements - elements - 1);
            UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT(*ptr_expected, *ptr_actual);
            UnityAddMsgIfSpecified(msg);
            UNITY_FAIL_AND_BAIL;
        }
        ptr_expected++;
        ptr_actual++;
    }
}

/*-----------------------------------------------*/
void UnityAssertDoublesWithin(const UNITY_DOUBLE delta,
                              const UNITY_DOUBLE expected,
                              const UNITY_DOUBLE actual,
                              const char* msg,
                              const UNITY_LINE_TYPE lineNumber)
{
    UNITY_SKIP_EXECUTION;

    if (!UnityDoublesWithin(delta, expected, actual))
    {
        UnityTestResultsFailBegin(lineNumber);
        UNITY_PRINT_EXPECTED_AND_ACTUAL_FLOAT(expected, actual);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/

void UnityAssertDoubleSpecial(const UNITY_DOUBLE actual,
                              const char* msg,
                              const UNITY_LINE_TYPE lineNumber,
                              const UNITY_FLOAT_TRAIT_T style)
{
    const char* trait_names[] = { UnityStrInf, UnityStrNegInf, UnityStrNaN, UnityStrDet };
    UNITY_INT should_be_trait   = ((UNITY_INT)style & 1);
    UNITY_INT is_trait          = !should_be_trait;
    UNITY_INT trait_index       = (UNITY_INT)(style >> 1);

    UNITY_SKIP_EXECUTION;

     switch(style)
    {
        case UNITY_FLOAT_IS_INF:
        case UNITY_FLOAT_IS_NOT_INF:
            is_trait = isinf(actual) & ispos(actual);
            break;
        case UNITY_FLOAT_IS_NEG_INF:
        case UNITY_FLOAT_IS_NOT_NEG_INF:
            is_trait = isinf(actual) & isneg(actual);
            break;

        case UNITY_FLOAT_IS_NAN:
        case UNITY_FLOAT_IS_NOT_NAN:
            is_trait = isnan(actual) ? 1 : 0;
            break;

        /* A determinate number is non infinite and not NaN. (therefore the opposite of the two above) */
        case UNITY_FLOAT_IS_DET:
        case UNITY_FLOAT_IS_NOT_DET:
            if (isinf(actual) || isnan(actual))
                is_trait = 0;
            else
                is_trait = 1;
            break;

        default:
            trait_index = 0;
            trait_names[0] = UnityStrInvalidFloatTrait;
            break;
    }

    if (is_trait != should_be_trait)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrExpected);
        if (!should_be_trait)
            UnityPrint(UnityStrNot);
        UnityPrint(trait_names[trait_index]);
        UnityPrint(UnityStrWas);
#ifndef UNITY_EXCLUDE_FLOAT_PRINT
        UnityPrintFloat(actual);
#else
        if (should_be_trait)
            UnityPrint(UnityStrNot);
        UnityPrint(trait_names[trait_index]);
#endif
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}


#endif /* not UNITY_EXCLUDE_DOUBLE */

/*-----------------------------------------------*/
void UnityAssertNumbersWithin( const UNITY_UINT delta,
                               const UNITY_INT expected,
                               const UNITY_INT actual,
                               const char* msg,
                               const UNITY_LINE_TYPE lineNumber,
                               const UNITY_DISPLAY_STYLE_T style)
{
    UNITY_SKIP_EXECUTION;

    if ((style & UNITY_DISPLAY_RANGE_INT) == UNITY_DISPLAY_RANGE_INT)
    {
        if (actual > expected)
            Unity.CurrentTestFailed = ((UNITY_UINT)(actual - expected) > delta);
        else
            Unity.CurrentTestFailed = ((UNITY_UINT)(expected - actual) > delta);
    }
    else
    {
        if ((UNITY_UINT)actual > (UNITY_UINT)expected)
            Unity.CurrentTestFailed = ((UNITY_UINT)(actual - expected) > delta);
        else
            Unity.CurrentTestFailed = ((UNITY_UINT)(expected - actual) > delta);
    }

    if (Unity.CurrentTestFailed)
    {
        UnityTestResultsFailBegin(lineNumber);
        UnityPrint(UnityStrDelta);
        UnityPrintNumberByStyle((UNITY_INT)delta, style);
        UnityPrint(UnityStrExpected);
        UnityPrintNumberByStyle(expected, style);
        UnityPrint(UnityStrWas);
        UnityPrintNumberByStyle(actual, style);
        UnityAddMsgIfSpecified(msg);
        UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/
void UnityAssertEqualString(const char* expected,
                            const char* actual,
                            const char* msg,
                            const UNITY_LINE_TYPE lineNumber)
{
    UNITY_UINT32 i;

    UNITY_SKIP_EXECUTION;

    /* if both pointers not null compare the strings */
    if (expected && actual)
    {
        for (i = 0; expected[i] || actual[i]; i++)
        {
            if (expected[i] != actual[i])
            {
                Unity.CurrentTestFailed = 1;
                break;
            }
        }
    }
    else
    { /* handle case of one pointers being null (if both null, test should pass) */
        if (expected != actual)
        {
            Unity.CurrentTestFailed = 1;
        }
    }

    if (Unity.CurrentTestFailed)
    {
      UnityTestResultsFailBegin(lineNumber);
      UnityPrintExpectedAndActualStrings(expected, actual);
      UnityAddMsgIfSpecified(msg);
      UNITY_FAIL_AND_BAIL;
    }
}

/*-----------------------------------------------*/
void UnityAssertEqualStringLen(const char* expected,
                            const char* actual,
                            const UNITY_UINT32 length,
                            const char* msg,
                            const UNITY_LINE_TYPE lineNumber)
{
    UNITY_UINT32 i;

    UNITY_SKIP_EXECUTION;

    /* if both pointers not null compare the strings */
    if (expected && actual)
    {
        for (i = 0; (i < length) && (expected[i] || actual[i]); i++)
        {
            if (expected[i] != actual[i])
            {
                Unity.CurrentTestFailed = 1;
                break;
            }
        }
    }
    else
    { /* handle case of one pointers being null (if both null, test should pass) */
        if (expected != actual)
        {
            Unity.CurrentTestFailed = 1;
        }
    }

    if (Unity.CurrentTestFailed)
    {
      UnityTestResultsFailBegin(lineNumber);
      UnityPrintExpectedAndActualStringsLen(expected, actual, length);
      UnityAddMsgIfSpecified(msg);
      UNITY_FAIL_AND_BAIL;
    }
}


/*-----------------------------------------------*/
void UnityAssertEqualStringArray( const char** expected,
                                  const char** actual,
                                  const UNITY_UINT32 num_elements,
                                  const char* msg,
                                  const UNITY_LINE_TYPE lineNumber)
{
    UNITY_UINT32 i, j = 0;

    UNITY_SKIP_EXECUTION;

    /* if no elements, it's an error */
    if (num_elements == 0)
    {
        UnityPrintPointlessAndBail();
    }

    if (UnityCheckArraysForNull((UNITY_INTERNAL_PTR)expected, (UNITY_INTERNAL_PTR)actual, lineNumber, msg) == 1)
        return;

    do
    {
        /* if both pointers not null compare the strings */
        if (expected[j] && actual[j])
        {
            for (i = 0; expected[j][i] || actual[j][i]; i++)
            {
                if (expected[j][i] != actual[j][i])
                {
                    Unity.CurrentTestFailed = 1;
                    break;
                }
            }
        }
        else
        { /* handle case of one pointers being null (if both null, test should pass) */
            if (expected[j] != actual[j])
            {
                Unity.CurrentTestFailed = 1;
            }
        }

        if (Unity.CurrentTestFailed)
        {
            UnityTestResultsFailBegin(lineNumber);
            if (num_elements > 1)
            {
                UnityPrint(UnityStrElement);
                UnityPrintNumberUnsigned(j);
            }
            UnityPrintExpectedAndActualStrings((const char*)(expected[j]), (const char*)(actual[j]));
            UnityAddMsgIfSpecified(msg);
            UNITY_FAIL_AND_BAIL;
        }
    } while (++j < num_elements);
}

/*-----------------------------------------------*/
void UnityAssertEqualMemory( UNITY_INTERNAL_PTR expected,
                             UNITY_INTERNAL_PTR actual,
                             const UNITY_UINT32 length,
                             const UNITY_UINT32 num_elements,
                             const char* msg,
                             const UNITY_LINE_TYPE lineNumber)
{
    UNITY_PTR_ATTRIBUTE const unsigned char* ptr_exp = (UNITY_PTR_ATTRIBUTE const unsigned char*)expected;
    UNITY_PTR_ATTRIBUTE const unsigned char* ptr_act = (UNITY_PTR_ATTRIBUTE const unsigned char*)actual;
    UNITY_UINT32 elements = num_elements;
    UNITY_UINT32 bytes;

    UNITY_SKIP_EXECUTION;

    if ((elements == 0) || (length == 0))
    {
        UnityPrintPointlessAndBail();
    }

    if (UnityCheckArraysForNull((UNITY_INTERNAL_PTR)expected, (UNITY_INTERNAL_PTR)actual, lineNumber, msg) == 1)
        return;

    while (elements--)
    {
        bytes = length;
        while (bytes--)
        {
            if (*ptr_exp != *ptr_act)
            {
                UnityTestResultsFailBegin(lineNumber);
                UnityPrint(UnityStrMemory);
                if (num_elements > 1)
                {
                    UnityPrint(UnityStrElement);
                    UnityPrintNumberUnsigned(num_elements - elements - 1);
                }
                UnityPrint(UnityStrByte);
                UnityPrintNumberUnsigned(length - bytes - 1);
                UnityPrint(UnityStrExpected);
                UnityPrintNumberByStyle(*ptr_exp, UNITY_DISPLAY_STYLE_HEX8);
                UnityPrint(UnityStrWas);
                UnityPrintNumberByStyle(*ptr_act, UNITY_DISPLAY_STYLE_HEX8);
                UnityAddMsgIfSpecified(msg);
                UNITY_FAIL_AND_BAIL;
            }
            ptr_exp = (UNITY_INTERNAL_PTR)((UNITY_PTR)ptr_exp + 1);
            ptr_act = (UNITY_INTERNAL_PTR)((UNITY_PTR)ptr_act + 1);
        }
    }
}

/*-----------------------------------------------
 * Control Functions
 *-----------------------------------------------*/

void UnityFail(const char* msg, const UNITY_LINE_TYPE line)
{
    UNITY_SKIP_EXECUTION;

    UnityTestResultsBegin(Unity.TestFile, line);
    UnityPrintFail();
    if (msg != NULL)
    {
        UNITY_OUTPUT_CHAR(':');

#ifndef UNITY_EXCLUDE_DETAILS
        if (Unity.CurrentDetail1)
        {
            UnityPrint(UnityStrDetail1Name);
            UnityPrint(Unity.CurrentDetail1);
            if (Unity.CurrentDetail2)
            {
                UnityPrint(UnityStrDetail2Name);
                UnityPrint(Unity.CurrentDetail2);
            }
            UnityPrint(UnityStrSpacer);
        }
#endif
        if (msg[0] != ' ')
        {
            UNITY_OUTPUT_CHAR(' ');
        }
        UnityPrint(msg);
    }

    UNITY_FAIL_AND_BAIL;
}

/*-----------------------------------------------*/
void UnityIgnore(const char* msg, const UNITY_LINE_TYPE line)
{
    UNITY_SKIP_EXECUTION;

    UnityTestResultsBegin(Unity.TestFile, line);
    UnityPrint(UnityStrIgnore);
    if (msg != NULL)
    {
      UNITY_OUTPUT_CHAR(':');
      UNITY_OUTPUT_CHAR(' ');
      UnityPrint(msg);
    }
    UNITY_IGNORE_AND_BAIL;
}

/*-----------------------------------------------*/
#if defined(UNITY_WEAK_ATTRIBUTE)
    UNITY_WEAK_ATTRIBUTE void setUp(void) { }
    UNITY_WEAK_ATTRIBUTE void tearDown(void) { }
#elif defined(UNITY_WEAK_PRAGMA)
#   pragma weak setUp
    void setUp(void) { }
#   pragma weak tearDown
    void tearDown(void) { }
#endif
/*-----------------------------------------------*/
void UnityDefaultTestRun(UnityTestFunction Func, const char* FuncName, const int FuncLineNum)
{
    Unity.CurrentTestName = FuncName;
    Unity.CurrentTestLineNumber = (UNITY_LINE_TYPE)FuncLineNum;
    Unity.NumberOfTests++;
    UNITY_CLR_DETAILS();
    if (TEST_PROTECT())
    {
        setUp();
        Func();
    }
    if (TEST_PROTECT() && !(Unity.CurrentTestIgnored))
    {
        tearDown();
    }
    UnityConcludeTest();
}

/*-----------------------------------------------*/
void UnityBegin(const char* filename)
{
    Unity.TestFile = filename;
    Unity.CurrentTestName = NULL;
    Unity.CurrentTestLineNumber = 0;
    Unity.NumberOfTests = 0;
    Unity.TestFailures = 0;
    Unity.TestIgnores = 0;
    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;

    UNITY_CLR_DETAILS();
    UNITY_OUTPUT_START();
}

/*-----------------------------------------------*/
int UnityEnd(void)
{
    UNITY_PRINT_EOL();
    UnityPrint(UnityStrBreaker);
    UNITY_PRINT_EOL();
    UnityPrintNumber((UNITY_INT)(Unity.NumberOfTests));
    UnityPrint(UnityStrResultsTests);
    UnityPrintNumber((UNITY_INT)(Unity.TestFailures));
    UnityPrint(UnityStrResultsFailures);
    UnityPrintNumber((UNITY_INT)(Unity.TestIgnores));
    UnityPrint(UnityStrResultsIgnored);
    UNITY_PRINT_EOL();
    if (Unity.TestFailures == 0U)
    {
        UnityPrintOk();
    }
    else
    {
        UnityPrintFail();
#ifdef UNITY_DIFFERENTIATE_FINAL_FAIL
        UNITY_OUTPUT_CHAR('E'); UNITY_OUTPUT_CHAR('D');
#endif
    }
    UNITY_PRINT_EOL();
    UNITY_FLUSH_CALL();
    UNITY_OUTPUT_COMPLETE();
    return (int)(Unity.TestFailures);
}

/*-----------------------------------------------
 * Command Line Argument Support
 *-----------------------------------------------*/
#ifdef UNITY_USE_COMMAND_LINE_ARGS

char* UnityOptionIncludeNamed = NULL;
char* UnityOptionExcludeNamed = NULL;
int   UnityVerbosity          = 1;

int UnityParseOptions(int argc, char** argv)
{
    UnityOptionIncludeNamed = NULL;
    UnityOptionExcludeNamed = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            switch(argv[i][1])
            {
                case 'l': /* list tests */
                    return -1;
                case 'n': /* include tests with name including this string */
                case 'f': /* an alias for -n */
                    if (argv[i][2] == '=')
                        UnityOptionIncludeNamed = &argv[i][3];
                    else if (++i < argc)
                        UnityOptionIncludeNamed = argv[i];
                    else
                    {
                        UnityPrint("ERROR: No Test String to Include Matches For");
                        UNITY_PRINT_EOL();
                        return 1;
                    }
                    break;
                case 'q': /* quiet */
                    UnityVerbosity = 0;
                    break;
                case 'v': /* verbose */
                    UnityVerbosity = 2;
                    break;
                case 'x': /* exclude tests with name including this string */
                    if (argv[i][2] == '=')
                        UnityOptionExcludeNamed = &argv[i][3];
                    else if (++i < argc)
                        UnityOptionExcludeNamed = argv[i];
                    else
                    {
                        UnityPrint("ERROR: No Test String to Exclude Matches For");
                        UNITY_PRINT_EOL();
                        return 1;
                    }
                    break;
                default:
                    UnityPrint("ERROR: Unknown Option ");
                    UNITY_OUTPUT_CHAR(argv[i][1]);
                    UNITY_PRINT_EOL();
                    return 1;
            }
        }
    }

    return 0;
}

int IsStringInBiggerString(const char* longstring, const char* shortstring)
{
    char* lptr = (char*)longstring;
    char* sptr = (char*)shortstring;
    char* lnext = lptr;

    if (*sptr == '*')
        return 1;

    while (*lptr)
    {
        lnext = lptr + 1;

        /* If they current bytes match, go on to the next bytes */
        while (*lptr && *sptr && (*lptr == *sptr))
        {
            lptr++;
            sptr++;

            /* We're done if we match the entire string or up to a wildcard */
            if (*sptr == '*')
                return 1;
            if (*sptr == ',')
                return 1;
            if (*sptr == '"')
                return 1;
            if (*sptr == '\'')
                return 1;
            if (*sptr == ':')
                return 2;
            if (*sptr == 0)
                return 1;
        }

        /* Otherwise we start in the long pointer 1 character further and try again */
        lptr = lnext;
        sptr = (char*)shortstring;
    }
    return 0;
}

int UnityStringArgumentMatches(const char* str)
{
    int retval;
    const char* ptr1;
    const char* ptr2;
    const char* ptrf;

    /* Go through the options and get the substrings for matching one at a time */
    ptr1 = str;
    while (ptr1[0] != 0)
    {
        if ((ptr1[0] == '"') || (ptr1[0] == '\''))
            ptr1++;

        /* look for the start of the next partial */
        ptr2 = ptr1;
        ptrf = 0;
        do {
            ptr2++;
            if ((ptr2[0] == ':') && (ptr2[1] != 0) && (ptr2[0] != '\'') && (ptr2[0] != '"') && (ptr2[0] != ','))
                ptrf = &ptr2[1];
        } while ((ptr2[0] != 0) && (ptr2[0] != '\'') && (ptr2[0] != '"') && (ptr2[0] != ','));
        while ((ptr2[0] != 0) && ((ptr2[0] == ':') || (ptr2[0] == '\'') || (ptr2[0] == '"') || (ptr2[0] == ',')))
            ptr2++;

        /* done if complete filename match */
        retval = IsStringInBiggerString(Unity.TestFile, ptr1);
        if (retval == 1)
            return retval;

        /* done if testname match after filename partial match */
        if ((retval == 2) && (ptrf != 0))
        {
            if (IsStringInBiggerString(Unity.CurrentTestName, ptrf))
                return 1;
        }

        /* done if complete testname match */
        if (IsStringInBiggerString(Unity.CurrentTestName, ptr1) == 1)
            return 1;

        ptr1 = ptr2;
    }

    /* we couldn't find a match for any substrings */
    return 0;
}

int UnityTestMatches(void)
{
    /* Check if this test name matches the included test pattern */
    int retval;
    if (UnityOptionIncludeNamed)
    {
        retval = UnityStringArgumentMatches(UnityOptionIncludeNamed);
    }
    else
        retval = 1;

    /* Check if this test name matches the excluded test pattern */
    if (UnityOptionExcludeNamed)
    {
        if (UnityStringArgumentMatches(UnityOptionExcludeNamed))
            retval = 0;
    }
    return retval;
}

#endif /* UNITY_USE_COMMAND_LINE_ARGS */
/*-----------------------------------------------*/
