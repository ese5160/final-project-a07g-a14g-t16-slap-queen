/**************************************************************************/ /**
 * @file      CliThread.c
 * @brief     File for the CLI Thread handler. Uses FREERTOS + CLI
 * @author    Eduardo Garcia
 * @date      2020-02-15

 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "CliThread.h"
#include "task.h"  // For xTaskGetTickCount

/******************************************************************************
 * Defines
 ******************************************************************************/
#define FIRMWARE_VERSION   "0.0.1"

/******************************************************************************
 * Variables
 ******************************************************************************/
extern SemaphoreHandle_t xRxSemaphore;

static int8_t *const pcWelcomeMessage =
    "FreeRTOS CLI.\r\nType Help to view a list of registered commands.\r\n";

// Clear screen command
const CLI_Command_Definition_t xClearScreen =
    {
        CLI_COMMAND_CLEAR_SCREEN,
        CLI_HELP_CLEAR_SCREEN,
        CLI_CALLBACK_CLEAR_SCREEN,
        CLI_PARAMS_CLEAR_SCREEN};

static const CLI_Command_Definition_t xResetCommand =
    {
        "reset",
        "reset: Resets the device\r\n",
        (const pdCOMMAND_LINE_CALLBACK)CLI_ResetDevice,
        0};

/* Added new command definitions */
static const CLI_Command_Definition_t xVersionCommand =
{
    "version",
    "version: Prints firmware version.\r\n",
    CLI_VersionCommand,
    0
};

static const CLI_Command_Definition_t xTicksCommand =
{
    "ticks",
    "ticks: Prints the number of RTOS ticks since startup.\r\n",
    CLI_TicksCommand,
    0
};

/******************************************************************************
 * Forward Declarations
 ******************************************************************************/
static void FreeRTOS_read(char *character);

/******************************************************************************
 * Callback Functions
 ******************************************************************************/

/******************************************************************************
 * CLI Thread
 ******************************************************************************/
void vCommandConsoleTask(void *pvParameters)
{
    // REGISTER COMMANDS HERE

    FreeRTOS_CLIRegisterCommand(&xClearScreen);
    FreeRTOS_CLIRegisterCommand(&xResetCommand);

    /* Register new commands */
    FreeRTOS_CLIRegisterCommand(&xVersionCommand);
    FreeRTOS_CLIRegisterCommand(&xTicksCommand);

    uint8_t cRxedChar[2], cInputIndex = 0;
    BaseType_t xMoreDataToFollow;
    /* The input and output buffers are declared static to keep them off the stack. */
    static char pcOutputString[MAX_OUTPUT_LENGTH_CLI], pcInputString[MAX_INPUT_LENGTH_CLI];
    static char pcLastCommand[MAX_INPUT_LENGTH_CLI];
    static bool isEscapeCode = false;
    static char pcEscapeCodes[4];
    static uint8_t pcEscapeCodePos = 0;

    // Any semaphores/mutexes/etc you needed to be initialized, you can do them here

    /* Send a welcome message to the user knows they are connected. */
    SerialConsoleWriteString(pcWelcomeMessage);
    char rxChar;
    for (;;)
    {
        FreeRTOS_read((char *)cRxedChar);

        if (cRxedChar[0] == '\n' || cRxedChar[0] == '\r')
        {
            SerialConsoleWriteString("\r\n");
            isEscapeCode = false;
            pcEscapeCodePos = 0;
            strncpy(pcLastCommand, pcInputString, MAX_INPUT_LENGTH_CLI - 1);
            pcLastCommand[MAX_INPUT_LENGTH_CLI - 1] = 0;

            do
            {
                xMoreDataToFollow = FreeRTOS_CLIProcessCommand(
                    pcInputString, 
                    pcOutputString, 
                    MAX_OUTPUT_LENGTH_CLI
                );

                pcOutputString[MAX_OUTPUT_LENGTH_CLI - 1] = 0;
                SerialConsoleWriteString(pcOutputString);

            } while (xMoreDataToFollow != pdFALSE);

            cInputIndex = 0;
            memset(pcInputString, 0x00, MAX_INPUT_LENGTH_CLI);
        }
        else
        {
            if (true == isEscapeCode)
            {
                if (pcEscapeCodePos < CLI_PC_ESCAPE_CODE_SIZE)
                {
                    pcEscapeCodes[pcEscapeCodePos++] = cRxedChar[0];
                }
                else
                {
                    isEscapeCode = false;
                    pcEscapeCodePos = 0;
                }

                if (pcEscapeCodePos >= CLI_PC_MIN_ESCAPE_CODE_SIZE)
                {
                    if (strcasecmp(pcEscapeCodes, "oa"))
                    {
                        sprintf(pcInputString, "%c[2K\r>", 27);
                        SerialConsoleWriteString(pcInputString);
                        cInputIndex = 0;
                        memset(pcInputString, 0x00, MAX_INPUT_LENGTH_CLI);
                        strncpy(pcInputString, pcLastCommand, MAX_INPUT_LENGTH_CLI - 1);
                        cInputIndex = (strlen(pcInputString) < MAX_INPUT_LENGTH_CLI - 1) ? strlen(pcInputString) : (MAX_INPUT_LENGTH_CLI - 1);
                        SerialConsoleWriteString(pcInputString);
                    }
                    isEscapeCode = false;
                    pcEscapeCodePos = 0;
                }
            }
            else if (cRxedChar[0] == '\r')
            {
            }
            else if (cRxedChar[0] == ASCII_BACKSPACE || cRxedChar[0] == ASCII_DELETE)
            {
                char erase[4] = {0x08, 0x20, 0x08, 0x00};
                SerialConsoleWriteString(erase);
                if (cInputIndex > 0)
                {
                    cInputIndex--;
                    pcInputString[cInputIndex] = 0;
                }
            }
            else if (cRxedChar[0] == ASCII_ESC)
            {
                isEscapeCode = true;
                pcEscapeCodePos = 0;
            }
            else
            {
                if (cInputIndex < MAX_INPUT_LENGTH_CLI)
                {
                    pcInputString[cInputIndex] = cRxedChar[0];
                    cInputIndex++;
                }
                cRxedChar[1] = 0;
                SerialConsoleWriteString((char *)&cRxedChar[0]);
            }
        }
    }
}

/**
 * @fn      static void FreeRTOS_read(char *character)
 * @brief   Blocks the CLI thread until a character is available in the ring buffer.
 * @note    Students must fill out this function to suspend until data arrives.
 *          The old code had 'vTaskSuspend(NULL)', which should be removed.
 *****************************************************************************/
static void FreeRTOS_read(char *character)
{
    if (xSemaphoreTake(xRxSemaphore, portMAX_DELAY) == pdTRUE)
    {
        uint8_t tempChar = 0;
        if (SerialConsoleReadCharacter(&tempChar) == 0)
        {
            *character = (char)tempChar;
        }
        else
        {
            *character = '\0';
        }
    }
    else
    {
        *character = '\0';
    }
}

/******************************************************************************
 * CLI Functions - Define here
 ******************************************************************************/

static char bufCli[CLI_MSG_LEN];
BaseType_t xCliClearTerminalScreen(char *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    char clearScreen = ASCII_ESC;
    snprintf(bufCli, CLI_MSG_LEN - 1, "%c[2J", clearScreen);
    snprintf(pcWriteBuffer, xWriteBufferLen, bufCli);
    return pdFALSE;
}

// Example CLI Command. Resets system.
BaseType_t CLI_ResetDevice(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    system_reset();
    return pdFALSE;
}

// Example CLI Command. Prints firmware version.
BaseType_t CLI_VersionCommand(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    snprintf((char *)pcWriteBuffer, xWriteBufferLen, "Firmware version: %s\r\n", FIRMWARE_VERSION);
    return pdFALSE;
}

// Example CLI Command. Prints system ticks.
BaseType_t CLI_TicksCommand(int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString)
{
    TickType_t ticks = xTaskGetTickCount();
    snprintf((char *)pcWriteBuffer, xWriteBufferLen, "%lu\r\n", (unsigned long)ticks);
    return pdFALSE;
}
