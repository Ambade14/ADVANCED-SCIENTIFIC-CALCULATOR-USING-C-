#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
#include <complex>
#include <iostream>
#include <mysql_driver.h>
#include <mysql_connection.h>


#ifdef _WIN32
#include <windows.h>
#endif

extern "C" {
    __declspec(dllexport) double Sin(double value);
    __declspec(dllexport) double Cos(double value);
    __declspec(dllexport) double Tan(double value);
    __declspec(dllexport) double Log(double value);
    __declspec(dllexport) double Ln(double value);
    __declspec(dllexport) double Exp(double value);
    __declspec(dllexport) double Power(double baseValue, double exponent);
    __declspec(dllexport) std::complex<double> AddComplex(std::complex<double> a, std::complex<double> b);
    __declspec(dllexport) std::complex<double> SubtractComplex(std::complex<double> a, std::complex<double> b);
    __declspec(dllexport) std::complex<double> MultiplyComplex(std::complex<double> a, std::complex<double> b);
    __declspec(dllexport) std::complex<double> DivideComplex(std::complex<double> a, std::complex<double> b);
    __declspec(dllexport) double Mean(const std::vector<double>& values);
    __declspec(dllexport) double Median(std::vector<double> values);
    __declspec(dllexport) double StandardDeviation(const std::vector<double>& values);
    __declspec(dllexport) void SaveCalculation(const std::string& calculation);
}

void InitializeDatabase(const std::string& connectionString);
void DeleteOldCalculations();

// Trigonometric functions
double Sin(double value) { return sin(value); }
double Cos(double value) { return cos(value); }
double Tan(double value) { return tan(value); }

// Logarithmic functions
double Log(double value) { return log10(value); }
double Ln(double value) { return log(value); }

// Exponential functions
double Exp(double value) { return exp(value); }
double Power(double baseValue, double exponent) { return pow(baseValue, exponent); }

// Complex number operations
std::complex<double> AddComplex(std::complex<double> a, std::complex<double> b) { return a + b; }
std::complex<double> SubtractComplex(std::complex<double> a, std::complex<double> b) { return a - b; }
std::complex<double> MultiplyComplex(std::complex<double> a, std::complex<double> b) { return a * b; }
std::complex<double> DivideComplex(std::complex<double> a, std::complex<double> b) { return a / b; }

// Statistical functions
double Mean(const std::vector<double>& values) {
    if (values.empty()) throw std::invalid_argument("Mean: empty vector");
    return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

double Median(std::vector<double> values) {
    if (values.empty()) throw std::invalid_argument("Median: empty vector");
    size_t size = values.size();
    std::sort(values.begin(), values.end());
    if (size % 2 == 0) {
        return (values[size / 2 - 1] + values[size / 2]) / 2;
    } else {
        return values[size / 2];
    }
}

double StandardDeviation(const std::vector<double>& values) {
    if (values.empty()) throw std::invalid_argument("StandardDeviation: empty vector");
    double mean = Mean(values);
    double sum = std::accumulate(values.begin(), values.end(), 0.0, [mean](double a, double b) { return a + (b - mean) * (b - mean); });
    return sqrt(sum / (values.size() - 1));
}

// Database connection details
static const std::string dbHost = "localhost";
static const std::string dbName = "sc";
static const std::string dbUser = "root";
static const std::string dbPassword = "Mangesh@14";
static const int HistoryLimit = 50;

void InitializeDatabase(const std::string& connectionString) {
    // MySQL connection is initialized directly in SaveCalculation function.
}

void SaveCalculation(const std::string& calculation) {
    try {
        sql::mysql::MySQL_Driver *driver;
        sql::Connection *con;

        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", dbUser, dbPassword);
        con->setSchema(dbName);

        sql::PreparedStatement *pstmt;
        pstmt = con->prepareStatement("INSERT INTO CalculationHistory (Calculation, Timestamp) VALUES (?, NOW())");
        pstmt->setString(1, calculation);
        pstmt->execute();

        delete pstmt;

        DeleteOldCalculations();

        delete con;
    } catch (sql::SQLException &e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

void DeleteOldCalculations() {
    try {
        sql::mysql::MySQL_Driver *driver;
        sql::Connection *con;

        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", dbUser, dbPassword);
        con->setSchema(dbName);

        sql::PreparedStatement *pstmt;
        pstmt = con->prepareStatement(
            "DELETE FROM CalculationHistory "
            "WHERE Id NOT IN (SELECT Id FROM CalculationHistory ORDER BY Timestamp DESC LIMIT ?)"
        );
        pstmt->setInt(1, HistoryLimit);
        pstmt->execute();

        delete pstmt;
        delete con;
    } catch (sql::SQLException &e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
