#include <UnitTest++/UnitTest++.h>
#include "../src/UserInterface.h"
#include "../src/Logger.h"
#include "../src/ClientDB.h"
#include "../src/Authenticator.h"
#include "../src/VectorProcessor.h"

#include <fstream>
#include <filesystem>
#include <sstream>
#include <cstdint>

namespace fs = std::filesystem;
SUITE(UserInterfaceSuite) {
    TEST(DefaultValues_NoArgs) {
        UserInterface ui;
        
        CHECK_EQUAL(ui.getPort(), 33333);
        CHECK_EQUAL(ui.getDbFile(), "vcalc.conf");
        CHECK_EQUAL(ui.getLogFile(), "vcalc.log");
    }

    TEST(CustomParameters_Valid) {
        char* argv[] = {(char*)"server", (char*)"--port", (char*)"8080", 
                        (char*)"--db", (char*)"/tmp/db", (char*)"--log", (char*)"/tmp/log"};
        UserInterface ui;
        ui.readArgs(7, argv);
        
        CHECK_EQUAL(ui.getPort(), 8080);
        CHECK_EQUAL(ui.getDbFile(), "/tmp/db");
        CHECK_EQUAL(ui.getLogFile(), "/tmp/log");
    }

    TEST(InvalidPort_NonNumeric) {
        char* argv[] = {(char*)"server", (char*)"--port", (char*)"abc"};
        UserInterface ui;
        CHECK_THROW(ui.readArgs(3, argv), std::exception);
    }

    TEST(MissingFlagArgument_Ignored) {
        char* argv[] = {(char*)"server", (char*)"--db"};
        UserInterface ui;
        ui.readArgs(2, argv);
        CHECK_EQUAL(ui.getDbFile(), "vcalc.conf");
    }
}
struct LoggerFixture {
    std::string testLogFile = "/tmp/test_vcalc_basic.log";
    
    LoggerFixture() { if (fs::exists(testLogFile)) fs::remove(testLogFile); }
    ~LoggerFixture() { if (fs::exists(testLogFile)) fs::remove(testLogFile); }
    
    std::string readFileContent() {
        std::ifstream file(testLogFile);
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }
};

SUITE(LoggerSuite) {
    TEST_FIXTURE(LoggerFixture, LogInfo_Success) {
        Logger logger(testLogFile);
        logger.log("Test info message", Severity::INFO);
        
        std::string content = readFileContent();
        CHECK(content.find("[INFO]") != std::string::npos);
        CHECK(content.find("Test info message") != std::string::npos);
    }

    TEST_FIXTURE(LoggerFixture, LogCritical_Success) {
        Logger logger(testLogFile);
        logger.log("Critical error", Severity::CRITICAL);
        
        std::string content = readFileContent();
        CHECK(content.find("[CRITICAL]") != std::string::npos);
    }
	TEST_FIXTURE(LoggerFixture, LogWarning_Success) {
        Logger logger(testLogFile);
        logger.log("Disk space low", Severity::WARNING);
        std::string content = readFileContent();
        CHECK(content.find("[WARNING]") != std::string::npos);
    }

    TEST_FIXTURE(LoggerFixture, InvalidDirectory_FailsGracefully) {
        Logger logger("/nonexistent/deeply/nested/dir/test.log");
        logger.log("This should not crash", Severity::INFO);
        CHECK(true);
    }
}

struct ClientDBFixture {
    std::string testDbFile = "/tmp/test_clients_basic.conf";
    
    ClientDBFixture() { if (fs::exists(testDbFile)) fs::remove(testDbFile); }
    ~ClientDBFixture() { if (fs::exists(testDbFile)) fs::remove(testDbFile); }
    
    void writeDb(const std::string& content) {
        std::ofstream f(testDbFile, std::ios::binary);
        f << content;
        f.close();
    }
};

SUITE(ClientDBSuite) {
    TEST_FIXTURE(ClientDBFixture, LoadSuccess_TwoUsers) {
        writeDb("user:pass\nadmin:123");
        ClientDB db(testDbFile);
        
        CHECK(db.load());
        CHECK(db.userExists("user"));
        CHECK_EQUAL(db.getPassword("user"), "pass");
    }

    TEST_FIXTURE(ClientDBFixture, UserNotFound_ReturnsEmpty) {
        writeDb("user:pass");
        ClientDB db(testDbFile);
        db.load();
        
        CHECK(!db.userExists("nonexistent"));
        CHECK_EQUAL(db.getPassword("nonexistent"), "");
    }

    TEST_FIXTURE(ClientDBFixture, NonExistentFile_LoadFails) {
        ClientDB db("/tmp/nonexistent_xyz_12345.conf");
        CHECK(!db.load());
    }
}

struct AuthenticatorFixture {
    Authenticator auth;
};

SUITE(AuthenticatorSuite) {
    TEST_FIXTURE(AuthenticatorFixture, GenerateSalt_NonZero) {
        uint64_t salt = auth.generateSalt();
        CHECK(salt != 0ULL);
    }

    TEST_FIXTURE(AuthenticatorFixture, FormatSalt16_CorrectPadding) {
        std::string formatted = auth.formatSalt16(255);
        CHECK_EQUAL(formatted.length(), 16);
        CHECK_EQUAL(formatted, "00000000000000FF");
    }

    TEST_FIXTURE(AuthenticatorFixture, VerifyHash_Correct) {
        uint64_t salt = auth.generateSalt();
        std::string salt16 = auth.formatSalt16(salt);
        std::string password = "P@ssW0rd";
        std::string hash = auth.computeMD5Hash(salt16, password);
        
        CHECK(auth.verifyHash(salt16, password, hash));
    }

    TEST_FIXTURE(AuthenticatorFixture, VerifyHash_WrongPassword) {
        uint64_t salt = auth.generateSalt();
        std::string salt16 = auth.formatSalt16(salt);
        std::string hash = auth.computeMD5Hash(salt16, "correct");
        
        CHECK(!auth.verifyHash(salt16, "wrong", hash));
    }
	TEST_FIXTURE(AuthenticatorFixture, FormatSalt16_MaxValue) {
        std::string formatted = auth.formatSalt16(UINT64_MAX);
        CHECK_EQUAL(formatted.length(), 16);
        CHECK_EQUAL(formatted, "FFFFFFFFFFFFFFFF");
    }
	TEST_FIXTURE(AuthenticatorFixture, VerifyHash_EmptyPassword) {
        uint64_t salt = auth.generateSalt();
        std::string salt16 = auth.formatSalt16(salt);
        std::string hash = auth.computeMD5Hash(salt16, "");
        CHECK(!auth.verifyHash(salt16, "notempty", hash));
    }
}

SUITE(VectorProcessorSuite) {
    VectorProcessor vp;

    TEST(Average_Normal) {
        std::vector<uint16_t> vec = {10, 20, 30};
        CHECK_EQUAL(vp.average(vec), 20);
    }

    TEST(Saturate_OverflowUp) {
        std::vector<uint16_t> vec = {65535, 65535};
        CHECK_EQUAL(vp.average(vec), 65535);
    }

    TEST(Average_EmptyVector) {
        std::vector<uint16_t> vec = {};
        CHECK_EQUAL(vp.average(vec), 0);
    }
	TEST(Average_SingleElement) {
        std::vector<uint16_t> vec = {42};
        CHECK_EQUAL(vp.average(vec), 42);
    }
}

int main() {
    return UnitTest::RunAllTests();
}