
#include <iostream>
#include <sstream>

#include "sigproc/common/unittest/unittest.hpp"
#include "sigproc/framework/compositestream.hpp"

using namespace sigproc::framework;

SIGPROC_TEST(CompositeDecode_int64) {

    CompositeStream decoder;
    decoder.push("1474");
    decoder.close();
    std::stringstream ss;
    ss << *decoder.root();
    ASSERT_NOT_NULL(decoder.root());
    ASSERT_EQUAL(CompositeDataType::INT64, decoder.root()->type());
    ASSERT_STR_EQUAL("1474", ss.str());
}


SIGPROC_TEST(CompositeDecode_float64) {

    CompositeStream decoder;
    decoder.push("3.1415");
    decoder.close();
    std::stringstream ss;
    ss << *decoder.root();
    ASSERT_NOT_NULL(decoder.root());
    ASSERT_EQUAL(CompositeDataType::FLOAT64, decoder.root()->type());
    ASSERT_STR_EQUAL("3.1415", ss.str());
}

SIGPROC_TEST(CompositeDecode_string) {

    CompositeStream decoder;
    decoder.push("\"abc\"");
    decoder.close();
    std::stringstream ss;
    ss << *decoder.root();
    ASSERT_NOT_NULL(decoder.root());
    ASSERT_EQUAL(CompositeDataType::STRING, decoder.root()->type());
    ASSERT_STR_EQUAL("\"abc\"", ss.str());
}

SIGPROC_TEST(CompositeDecode_simple_seq) {

    CompositeStream decoder;
    decoder.push("[1, 2, 3]");
    decoder.close();
    std::stringstream ss;
    ss << *decoder.root();
    ASSERT_NOT_NULL(decoder.root());
    ASSERT_EQUAL(CompositeDataType::SEQ, decoder.root()->type());
    ASSERT_STR_EQUAL("[1, 2, 3]", ss.str());
}

SIGPROC_TEST(CompositeDecode_simple_map) {

    CompositeStream decoder;

    decoder.push("{ \"123\" : 1, \"456\" : 2 } ");
    decoder.close();
    std::stringstream ss;
    ss << *decoder.root();
    ASSERT_NOT_NULL(decoder.root());
    ASSERT_EQUAL(CompositeDataType::MAP, decoder.root()->type());
    // TODO: order may not be guaranteed...
    ASSERT_STR_EQUAL("{\"123\": 1, \"456\": 2}", ss.str());
}

SIGPROC_TEST(CompositeDecode_token_map) {

    CompositeStream decoder;

    decoder.push("{ test : 1 } ");
    decoder.close();
    std::stringstream ss;
    ss << *decoder.root();
    ASSERT_NOT_NULL(decoder.root());
    ASSERT_EQUAL(CompositeDataType::MAP, decoder.root()->type());
    // TODO: order may not be guaranteed...
    ASSERT_STR_EQUAL("{\"test\": 1}", ss.str());
}

SIGPROC_TEST(CompositeDecode_compact_map) {

    CompositeStream decoder;

    decoder.push("{\"123\":1}");
    decoder.close();
    std::stringstream ss;
    ss << *decoder.root();

    ASSERT_NOT_NULL(decoder.root());
    ASSERT_EQUAL(CompositeDataType::MAP, decoder.root()->type());
    // TODO: order may not be guaranteed...
    ASSERT_STR_EQUAL("{\"123\": 1}", ss.str());
}


SIGPROC_TEST(CompositeDecode_token) {

    {
        CompositeStream decoder;
        decoder.push("true");
        decoder.close();
        ASSERT_NOT_NULL(decoder.root());
        ASSERT_EQUAL(CompositeDataType::INT64, decoder.root()->type());
        int64_t* i=nullptr;
        decoder.root()->value<int64_t>(&i);
        int64_t e = 1;
        ASSERT_EQUAL(e, *i);
    }

    {
        CompositeStream decoder;
        decoder.push("false");
        decoder.close();
        ASSERT_NOT_NULL(decoder.root());
        ASSERT_EQUAL(CompositeDataType::INT64, decoder.root()->type());
        int64_t* i = nullptr;
        decoder.root()->value<int64_t>(&i);
        int64_t e = 0;
        ASSERT_EQUAL(e, *i);
    }

    {
        CompositeStream decoder;
        decoder.push("null");
        decoder.close();
        ASSERT_NOT_NULL(decoder.root());
        ASSERT_EQUAL(CompositeDataType::STRING, decoder.root()->type());
        char** s=nullptr;
        decoder.root()->value<char*>(&s);
        const char* e = "";
        ASSERT_STR_EQUAL(e, *s);
    }
}


SIGPROC_TEST(CompositeStreamDecode_map1) {

    CompositeStream decoder;
    decoder.push("{nodes: [{a:0},  {b:0},  {c:0}]}");
    decoder.close();

    if (decoder.root() != nullptr) {
        std::stringstream ss;
        ss << *decoder.root();
        ASSERT_STR_EQUAL("{\"nodes\": [{\"a\": 0}, {\"b\": 0}, {\"c\": 0}]}", ss.str());
    }
}

SIGPROC_TEST(CompositeStreamDecode_map2) {

    CompositeStream decoder;

    decoder.push("{ nodes:");
    if (decoder.root() != nullptr) {
        std::stringstream ss;
        ss << *decoder.root();
        ASSERT_STR_EQUAL("{}", ss.str());
    }

    decoder.push("[{a:0}, ");
    if (decoder.root() != nullptr) {
        std::stringstream ss;
        ss << *decoder.root();
        ASSERT_STR_EQUAL("{\"nodes\": [{\"a\": 0}]}", ss.str());
    }

    decoder.push(" { b : 0 } , ");
    if (decoder.root() != nullptr) {
        std::stringstream ss;
        ss << *decoder.root();
        ASSERT_STR_EQUAL("{\"nodes\": [{\"a\": 0}, {\"b\": 0}]}", ss.str());
    }

    decoder.push("{c:0}");
    if (decoder.root() != nullptr) {
        std::stringstream ss;
        ss << *decoder.root();
        ASSERT_STR_EQUAL("{\"nodes\": [{\"a\": 0}, {\"b\": 0}, {\"c\": 0}]}", ss.str());
    }

    decoder.push("]}");
    if (decoder.root() != nullptr) {
        std::stringstream ss;
        ss << *decoder.root();
        ASSERT_STR_EQUAL("{\"nodes\": [{\"a\": 0}, {\"b\": 0}, {\"c\": 0}]}", ss.str());
    }

    decoder.close();
    if (decoder.root() != nullptr) {
        std::stringstream ss;
        ss << *decoder.root();
        ASSERT_STR_EQUAL("{\"nodes\": [{\"a\": 0}, {\"b\": 0}, {\"c\": 0}]}", ss.str());
    }

}

/**
    keys in a JSON document for a map must be strings
*/
SIGPROC_TEST(CompositeStreamDecode_error_map_key) {

    CompositeStream decoder;

    try {
        decoder.push("{ 123 : 456 }");
        ASSERT_FAIL("expected throw");
    } catch (CompositeStreamException& ex) {

    }
}

/**
    keys in a JSON document for a map must be strings
*/
SIGPROC_TEST(CompositeStreamDecode_error_empty_set) {

    CompositeStream decoder;

    try {
        decoder.push("[ 1, , 3]");
        ASSERT_FAIL("expected throw");
    } catch (CompositeStreamException& ex) {

    }
}


SIGPROC_TEST(CompositeStreamDecode_error_unterminated_string) {

    CompositeStream decoder;

    decoder.push("\n\n \"help");

    try {
        decoder.close();
        ASSERT_FAIL("expected throw");
    } catch (CompositeStreamException& ex) {
        size_t expected = 2;
        ASSERT_EQUAL(expected, ex.line());
    }
}

SIGPROC_TEST(CompositeStreamDecode_error_unterminated_collection) {

    CompositeStream decoder;

    decoder.push("\n\n   { abc: 0, ");

    try {
        decoder.close();
        ASSERT_FAIL("expected throw");
    } catch (CompositeStreamException& ex) {
        size_t expected = 4;
        std::cout << ex.what() << std::endl;
        ASSERT_EQUAL(expected, ex.line());
    }
}

// TODO: close() close(true);
// validate the JSON and close the stream
