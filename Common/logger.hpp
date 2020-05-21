/**
 *  @file       logger.hpp
 *  @brief      An universal logger
 *  @author     Jozef Zuzelka <xzuzel00@stud.fit.vutbr.cz>
 *  @date
 *   - Created: 07.12.2018 16:58
 *   - Edited:  03.12.2019 22:22
 *  @version    1.0.0
 *  @par        g++: Apple LLVM version 10.0.0 (clang-1000.11.45.5)
 *  @bug
 *  @todo
 */

#pragma once

#include <mutex>
#include <iostream>
#include <atomic>



#ifdef DEBUG_BUILD

/*!
 * @brief       Debug that calls #printArray function
 * @param[in]   bitArray    Array to be printed
 * @param[in]   dataSize    Size of the array
 */
#define D_ARRAY(bitArray, dataSize) printArray(bitArray, dataSize)
/*!
 * @brief   Variadic debug macro that prints everything to the standard error output
 * @note    Arguments must be delimited with <<
 */
#define D(...) \
do { \
    std::lock_guard<std::mutex> guard(m_debugPrint); \
    std::cerr << "DEBUG: " << __FILE__ << ":" << __LINE__ << ":<" << RED << __func__ <<  CLR << ">: "; \
    std::cerr << __VA_ARGS__ << std::endl; \
} while (0)

#else   //  DEBUG_BUILD

/*!
 * @brief       Debug macro which is substituted to nothing
 * @param[in]   x   Array to be printed
 * @param[in]   y   Size of the array
 */
#define D_ARRAY(x,y)
 /*!
  * @brief   Variadic debug macro which is substituted to nothing
  */
#define D(...)

#endif  // DEBUG_BUILD
/*!
 * @enum    LogLevel
 * @brief   An enum representing debug prints verbosity
 */


enum class LogLevel : uint8_t
{
    DEBUG,      //!< Debug messages
    INFO,       //!< Informational messages
    WARNING,    //!< Warning messages
    ERR,        //!< Error messages
    NONE,       //!< Nothing is printed
};


//! Array of prefixes of debug messages
const char * const msgPrefix[] = {"[DD]", "[II]", "[WW]", "[EE]", ""};


/*!
 * @class Logger
 * @brief Class for logging
 */
class Logger
{
        std::atomic<LogLevel> m_logLevel;
        std::mutex m_debugPrint;

        Logger(const Logger&) = delete;
        Logger(LogLevel ll = LogLevel::WARNING) : m_logLevel(ll) { };
        ~Logger() {};

    public:
        /*!
         * @brief       Singleton pattern
         * @param[in]   ll  LogLevel
         * @return      A single instance of the logger
         */
        static Logger &getInstance()
        {
            static Logger logger;
            return logger;
        }

        /*!
         * @brief       Function prints array in hexadecimal
         * @param[in]   bitArray    Array to be printed
         * @param[in]   dataSize    Amount of data to be printed
         */
        inline void printArray(const unsigned char *bitArray, const unsigned int dataSize)
        {
            std::lock_guard<std::mutex> guard(m_debugPrint);
            std::cerr << "Data (" << dataSize << "): ";
            for (unsigned int i=0; i != dataSize; i++)
                std::cerr << std::hex << (bitArray[i]>>4) << (bitArray[i]&0x0f) << std::dec;
            std::cerr << std::endl;
        }

        /*!
         * @brief       Function that prints log messages
         * @param[in]   ll  Verbosity level
         * @todo        Improve param description
         * @param[in]   args    Variadic parametes
         */
        template <typename ... Ts>
        void log(LogLevel ll, Ts&&... args)
        {
            if (ll >= m_logLevel)
            {
                std::lock_guard<std::mutex> guard(m_debugPrint);
                std::cerr << msgPrefix[static_cast<int>(ll)] << " ";
                (std::cerr << ... << args) << std::endl;
            }
        }

        /*!
         * @brief       Sets m_logLevel;
         * @param[in]   ll  LogLevel
         */
        void setLogLevel(LogLevel ll)
        {
            m_logLevel = ll;
        }

        /*!
         * @brief       Sets m_logLevel;
         * @param[in]   ll  Integer value 0-4
         */
        void setLogLevel(std::string ll)
        {
            switch (stoi(ll))
            {
                case 0:
                    setLogLevel(LogLevel::NONE);
                    break;
                case 1:
                    setLogLevel(LogLevel::ERR);
                    break;
                case 2:
                    setLogLevel(LogLevel::WARNING);
                    break;
                case 3:
                    setLogLevel(LogLevel::INFO);
                    break;
                case 4:
                    setLogLevel(LogLevel::DEBUG);
                    break;
                default:
                    log(LogLevel::ERR, "Invalid verbosity level (", ll, ").");
                    setLogLevel(LogLevel::INFO);
            }
        }
};
