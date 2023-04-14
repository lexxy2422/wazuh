
#include <any>
#include <memory>
#include <vector>

#include <gtest/gtest.h>
#include <json/json.hpp>

#include <baseTypes.hpp>
#include <kvdb/kvdbManager.hpp>
#include <testsCommon.hpp>
#include <opBuilderKVDB.hpp>

#include <metrics/metricsManager.hpp>
using namespace metricsManager;

namespace
{
using namespace base;
namespace bld = builder::internals::builders;

class opBuilderKVDBGetTest : public ::testing::Test
{

protected:
    static constexpr auto DB_NAME = "TEST_DB";
    static constexpr auto DB_DIR = "/tmp/";

    std::shared_ptr<IMetricsManager> m_manager;
    std::shared_ptr<kvdb_manager::KVDBManager> kvdbManager;

    void SetUp() override
    {
        initLogging();

        m_manager = std::make_shared<MetricsManager>();
        kvdbManager = std::make_shared<kvdb_manager::KVDBManager>(opBuilderKVDBGetTest::DB_DIR, m_manager);

        auto res = kvdbManager->getHandler(DB_NAME, true);
        if (auto err = std::get_if<base::Error>(&res))
        {
            throw std::runtime_error(err->message);
        }
        auto db = std::get<kvdb_manager::KVDBHandle>(res);
    }

    void TearDown() override { kvdbManager->unloadDB(DB_NAME); }
};

// Build ok
TEST_F(opBuilderKVDBGetTest, BuildsGetI)
{
    auto tuple = std::make_tuple<std::string, std::string, std::vector<std::string>>("/field", "", {DB_NAME, "key"});

    ASSERT_NO_THROW(bld::KVDBGet(tuple, false, opBuilderKVDBGetTest::kvdbManager));
    ASSERT_NO_THROW(bld::KVDBGet(tuple, true, opBuilderKVDBGetTest::kvdbManager));
}

TEST_F(opBuilderKVDBGetTest, BuildsGetII)
{
    auto tuple = std::make_tuple<std::string, std::string, std::vector<std::string>>("/field", "", {DB_NAME, "$key"});

    ASSERT_NO_THROW(bld::KVDBGet(tuple, false, opBuilderKVDBGetTest::kvdbManager));
    ASSERT_NO_THROW(bld::KVDBGet(tuple, true, opBuilderKVDBGetTest::kvdbManager));
}

TEST_F(opBuilderKVDBGetTest, WrongNumberOfParameters)
{
    auto tuple = std::make_tuple<std::string, std::string, std::vector<std::string>>("/field", "", {DB_NAME});

    ASSERT_THROW(bld::KVDBGet(tuple, false, opBuilderKVDBGetTest::kvdbManager), std::runtime_error);
    ASSERT_THROW(bld::KVDBGet(tuple, true, opBuilderKVDBGetTest::kvdbManager), std::runtime_error);
}

TEST_F(opBuilderKVDBGetTest, GetSuccessCases)
{
    // Insert data in DB
    auto res = kvdbManager->getHandler(DB_NAME);
    if (auto err = std::get_if<base::Error>(&res))
    {
        throw std::runtime_error(err->message);
    }
    auto DBHandle = std::get<kvdb_manager::KVDBHandle>(res);
    DBHandle->write("keyString", R"("string_value")");
    DBHandle->write("keyNumber", "123");
    DBHandle->write("keyObject", R"({"field1": "value1", "field2": "value2"})");
    DBHandle->write("keyArray", R"(["value1", "value2"])");
    DBHandle->write("keyNull", "null");

    // Operations value key
    auto tuple1 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldString", "", {DB_NAME, "keyString"});
    auto op1 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple1);
    auto tuple2 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldNumber", "", {DB_NAME, "keyNumber"});
    auto op2 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple2);
    auto tuple3 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldObject", "", {DB_NAME, "keyObject"});
    auto op3 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple3);
    auto tuple4 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldArray", "", {DB_NAME, "keyArray"});
    auto op4 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple4);
    auto tuple5 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldNull", "", {DB_NAME, "keyNull"});
    auto op5 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple5);

    // Operations reference key
    auto tuple6 = std::make_tuple<std::string, std::string, std::vector<std::string>>(
        "/fieldString", "", {DB_NAME, "$keyString"});
    auto op6 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple6);
    auto tuple7 = std::make_tuple<std::string, std::string, std::vector<std::string>>(
        "/fieldNumber", "", {DB_NAME, "$keyNumber"});
    auto op7 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple7);
    auto tuple8 = std::make_tuple<std::string, std::string, std::vector<std::string>>(
        "/fieldObject", "", {DB_NAME, "$keyObject"});
    auto op8 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple8);
    auto tuple9 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldArray", "", {DB_NAME, "$keyArray"});
    auto op9 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple9);
    auto tuple10 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldNull", "", {DB_NAME, "$keyNull"});
    auto op10 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple10);

    // Events templates
    json::Json eventTemplate1 {R"({
        "fieldString": "value",
        "fieldNumber": 1,
        "fieldObject": {"field": "value"},
        "fieldArray": ["value"],
        "fieldNull": null,
        "keyString": "keyString",
        "keyNumber": "keyNumber",
        "keyObject": "keyObject",
        "keyArray": "keyArray",
        "keyNull": "keyNull"
    })"};
    json::Json eventTemplate2 {R"({
        "keyString": "keyString",
        "keyNumber": "keyNumber",
        "keyObject": "keyObject",
        "keyArray": "keyArray",
        "keyNull": "keyNull"
    })"};

    // Use case events
    auto event1_0 = std::make_shared<json::Json>(eventTemplate1);
    auto event1_1 = std::make_shared<json::Json>(eventTemplate2);
    auto event2_0 = std::make_shared<json::Json>(eventTemplate1);
    auto event2_1 = std::make_shared<json::Json>(eventTemplate2);
    auto event3_0 = std::make_shared<json::Json>(eventTemplate1);
    auto event3_1 = std::make_shared<json::Json>(eventTemplate2);
    auto event4_0 = std::make_shared<json::Json>(eventTemplate1);
    auto event4_1 = std::make_shared<json::Json>(eventTemplate2);
    auto event5_0 = std::make_shared<json::Json>(eventTemplate1);
    auto event5_1 = std::make_shared<json::Json>(eventTemplate2);
    auto event6_0 = std::make_shared<json::Json>(eventTemplate1);
    auto event6_1 = std::make_shared<json::Json>(eventTemplate2);
    auto event7_0 = std::make_shared<json::Json>(eventTemplate1);
    auto event7_1 = std::make_shared<json::Json>(eventTemplate2);
    auto event8_0 = std::make_shared<json::Json>(eventTemplate1);
    auto event8_1 = std::make_shared<json::Json>(eventTemplate2);
    auto event9_0 = std::make_shared<json::Json>(eventTemplate1);
    auto event9_1 = std::make_shared<json::Json>(eventTemplate2);
    auto event10_0 = std::make_shared<json::Json>(eventTemplate1);
    auto event10_1 = std::make_shared<json::Json>(eventTemplate2);

    // Use case expected events
    auto expectedEvent1_0 = std::make_shared<json::Json>(eventTemplate1);
    expectedEvent1_0->setString("string_value", "/fieldString");
    auto expectedEvent1_1 = std::make_shared<json::Json>(eventTemplate2);
    expectedEvent1_1->setString("string_value", "/fieldString");
    auto expectedEvent2_0 = std::make_shared<json::Json>(eventTemplate1);
    expectedEvent2_0->setInt(123, "/fieldNumber");
    auto expectedEvent2_1 = std::make_shared<json::Json>(eventTemplate2);
    expectedEvent2_1->setInt(123, "/fieldNumber");
    auto expectedEvent3_0 = std::make_shared<json::Json>(eventTemplate1);
    expectedEvent3_0->set("/fieldObject", json::Json {R"({"field1": "value1", "field2": "value2"})"});
    auto expectedEvent3_1 = std::make_shared<json::Json>(eventTemplate2);
    expectedEvent3_1->set("/fieldObject", json::Json {R"({"field1": "value1", "field2": "value2"})"});
    auto expectedEvent4_0 = std::make_shared<json::Json>(eventTemplate1);
    expectedEvent4_0->set("/fieldArray", json::Json {R"(["value1", "value2"])"});
    auto expectedEvent4_1 = std::make_shared<json::Json>(eventTemplate2);
    expectedEvent4_1->set("/fieldArray", json::Json {R"(["value1", "value2"])"});
    auto expectedEvent5_0 = std::make_shared<json::Json>(eventTemplate1);
    expectedEvent5_0->setNull("/fieldNull");
    auto expectedEvent5_1 = std::make_shared<json::Json>(eventTemplate2);
    expectedEvent5_1->setNull("/fieldNull");

    // Use cases string
    auto result = op1->getPtr<Term<EngineOp>>()->getFn()(event1_0);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent1_0);
    result = op1->getPtr<Term<EngineOp>>()->getFn()(event1_1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent1_1);
    result = op6->getPtr<Term<EngineOp>>()->getFn()(event6_0);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent1_0);
    result = op6->getPtr<Term<EngineOp>>()->getFn()(event6_1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent1_1);

    // Use cases number
    result = op2->getPtr<Term<EngineOp>>()->getFn()(event2_0);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent2_0);
    result = op2->getPtr<Term<EngineOp>>()->getFn()(event2_1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent2_1);
    result = op7->getPtr<Term<EngineOp>>()->getFn()(event7_0);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent2_0);
    result = op7->getPtr<Term<EngineOp>>()->getFn()(event7_1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent2_1);

    // Use cases object
    result = op3->getPtr<Term<EngineOp>>()->getFn()(event3_0);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent3_0);
    result = op3->getPtr<Term<EngineOp>>()->getFn()(event3_1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent3_1);
    result = op8->getPtr<Term<EngineOp>>()->getFn()(event8_0);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent3_0);
    result = op8->getPtr<Term<EngineOp>>()->getFn()(event8_1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent3_1);

    // Use cases array
    result = op4->getPtr<Term<EngineOp>>()->getFn()(event4_0);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent4_0);
    result = op4->getPtr<Term<EngineOp>>()->getFn()(event4_1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent4_1);
    result = op9->getPtr<Term<EngineOp>>()->getFn()(event9_0);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent4_0);
    result = op9->getPtr<Term<EngineOp>>()->getFn()(event9_1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent4_1);

    // Use cases null
    result = op5->getPtr<Term<EngineOp>>()->getFn()(event5_0);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent5_0);
    result = op5->getPtr<Term<EngineOp>>()->getFn()(event5_1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent5_1);
    result = op10->getPtr<Term<EngineOp>>()->getFn()(event10_0);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent5_0);
    result = op10->getPtr<Term<EngineOp>>()->getFn()(event10_1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent5_1);
}

TEST_F(opBuilderKVDBGetTest, GetFailKeyNotFound)
{
    auto tuple1 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/field", "", {DB_NAME, "NotFoundKey"});
    auto op1 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple1);
    auto tuple2 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/field", "", {DB_NAME, "$NotFoundKey"});
    auto op2 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple2);
    auto tuple3 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/field", "", {DB_NAME, "$fieldNotFound"});
    auto op3 = bld::getOpBuilderKVDBGet(kvdbManager)(tuple3);

    auto event = std::make_shared<json::Json>(R"({
        "NotFoundKey": "NotFoundKey"
    })");

    auto result = op1->getPtr<Term<EngineOp>>()->getFn()(event);
    ASSERT_FALSE(result);
    result = op2->getPtr<Term<EngineOp>>()->getFn()(event);
    ASSERT_FALSE(result);
    result = op3->getPtr<Term<EngineOp>>()->getFn()(event);
    ASSERT_FALSE(result);
}

TEST_F(opBuilderKVDBGetTest, GetMergeSuccessCases)
{
    // Insert data in DB
    auto res = kvdbManager->getHandler(DB_NAME);
    if (auto err = std::get_if<base::Error>(&res))
    {
        throw std::runtime_error(err->message);
    }
    auto DBHandle = std::get<kvdb_manager::KVDBHandle>(res);
    DBHandle->write("keyObject", R"({"field1": "value1", "field2": "value2", "field3": "value3"})");
    DBHandle->write("keyArray", R"(["value1", "value2", "value3"])");

    // Operations value key
    auto tuple1 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldObject", "", {DB_NAME, "keyObject"});
    auto op1 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple1);
    auto tuple2 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldArray", "", {DB_NAME, "keyArray"});
    auto op2 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple2);

    // Operations reference key
    auto tuple3 = std::make_tuple<std::string, std::string, std::vector<std::string>>(
        "/fieldObject", "", {DB_NAME, "$keyObject"});
    auto op3 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple3);
    auto tuple4 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldArray", "", {DB_NAME, "$keyArray"});
    auto op4 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple4);

    // Events templates
    json::Json eventTemplate {R"({
        "fieldObject": {"field2": "value_old"},
        "fieldArray": ["value2"],
        "keyObject": "keyObject",
        "keyArray": "keyArray"
    })"};

    // Use case events
    auto event1 = std::make_shared<json::Json>(eventTemplate);
    auto event2 = std::make_shared<json::Json>(eventTemplate);
    auto event3 = std::make_shared<json::Json>(eventTemplate);
    auto event4 = std::make_shared<json::Json>(eventTemplate);

    // Use case expected events
    auto expectedEvent1 = std::make_shared<json::Json>(eventTemplate);
    expectedEvent1->set("/fieldObject", json::Json {R"({"field2": "value2", "field1": "value1", "field3": "value3"})"});
    auto expectedEvent2 = std::make_shared<json::Json>(eventTemplate);
    expectedEvent2->set("/fieldArray", json::Json {R"(["value2", "value1", "value3"])"});

    // Use cases object
    auto result = op1->getPtr<Term<EngineOp>>()->getFn()(event1);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent1);
    result = op3->getPtr<Term<EngineOp>>()->getFn()(event3);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent1);

    // Use cases array
    result = op2->getPtr<Term<EngineOp>>()->getFn()(event2);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent2);
    result = op4->getPtr<Term<EngineOp>>()->getFn()(event4);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result.payload(), *expectedEvent2);
}

TEST_F(opBuilderKVDBGetTest, GetMergeFailKeyNotFound)
{
    auto tuple1 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/field", "", {DB_NAME, "NotFoundKey"});
    auto op1 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple1);
    auto tuple2 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/field", "", {DB_NAME, "$NotFoundKey"});
    auto op2 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple2);
    auto tuple3 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/field", "", {DB_NAME, "$fieldNotFound"});
    auto op3 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple3);

    auto event = std::make_shared<json::Json>(R"({
        "NotFoundKey": "NotFoundKey"
    })");

    auto result = op1->getPtr<Term<EngineOp>>()->getFn()(event);
    ASSERT_FALSE(result);
    result = op2->getPtr<Term<EngineOp>>()->getFn()(event);
    ASSERT_FALSE(result);
    result = op3->getPtr<Term<EngineOp>>()->getFn()(event);
    ASSERT_FALSE(result);
}

TEST_F(opBuilderKVDBGetTest, GetMergeFailTargetNotFound)
{
    // Insert data in DB
    auto res = kvdbManager->getHandler(DB_NAME);
    if (auto err = std::get_if<base::Error>(&res))
    {
        throw std::runtime_error(err->message);
    }
    auto DBHandle = std::get<kvdb_manager::KVDBHandle>(res);
    DBHandle->write("keyObject", R"({"field1": "value1", "field2": "value2", "field3": "value3"})");

    auto tuple1 = std::make_tuple<std::string, std::string, std::vector<std::string>>(
        "/fieldNotFound", "", {DB_NAME, "keyObject"});
    auto op1 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple1);
    auto tuple2 = std::make_tuple<std::string, std::string, std::vector<std::string>>(
        "/fieldNotFound", "", {DB_NAME, "$keyObject"});
    auto op2 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple2);

    auto event = std::make_shared<json::Json>(R"({
        "keyObject": "keyObject"
    })");

    auto result = op1->getPtr<Term<EngineOp>>()->getFn()(event);
    ASSERT_FALSE(result);
    result = op2->getPtr<Term<EngineOp>>()->getFn()(event);
    ASSERT_FALSE(result);
}

TEST_F(opBuilderKVDBGetTest, GetMergeFailTypeErrors)
{
    // Insert data in DB
    auto res = kvdbManager->getHandler(DB_NAME);
    if (auto err = std::get_if<base::Error>(&res))
    {
        throw std::runtime_error(err->message);
    }
    auto DBHandle = std::get<kvdb_manager::KVDBHandle>(res);
    DBHandle->write("keyObject", R"({"field1": "value1", "field2": "value2", "field3": "value3"})");
    DBHandle->write("keyArray", R"(["value1", "value2", "value3"])");
    DBHandle->write("keyString", R"("value1")");

    auto tuple1 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldObject", "", {DB_NAME, "keyArray"});
    auto op1 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple1);
    auto tuple2 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldArray", "", {DB_NAME, "keyObject"});
    auto op2 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple2);
    auto tuple3 =
        std::make_tuple<std::string, std::string, std::vector<std::string>>("/fieldString", "", {DB_NAME, "keyString"});
    auto op3 = bld::getOpBuilderKVDBGetMerge(kvdbManager)(tuple3);

    auto event = std::make_shared<json::Json>(R"({
        "fieldObject": {"key": "value"},
        "fieldArray": ["value"],
        "fieldString": "value"
    })");

    auto result = op1->getPtr<Term<EngineOp>>()->getFn()(event);
    ASSERT_FALSE(result);
    result = op2->getPtr<Term<EngineOp>>()->getFn()(event);
    ASSERT_FALSE(result);
    result = op3->getPtr<Term<EngineOp>>()->getFn()(event);
    ASSERT_FALSE(result);
}
} // namespace
