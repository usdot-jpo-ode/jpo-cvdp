# Troubleshooting
This document is intended to assist developers with troubleshooting the PPM.

## Contents
- [How to stringify a variable of type rapidjson::Value&](#How-to-stringify-a-variable-of-type-rapidjson::Value&)
- [Error: Assertion 'IsObject()' failed - rapidjson](#Error:-Assertion-'IsObject()'-failed---rapidjson)
- [Error: No CMAKE_CXX_Compiler could be found](#Error:-No-CMAKE_CXX_Compiler-could-be-found)

## How to stringify a variable of type rapidjson::Value&
This involves using rapidjson's StringBuffer and Writer classes. This can come in handy when troubleshooting tests for the BSM Filter.

### Example Code
    rapidjson::Value& example = data["example"];
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    example.Accept(writer);
    cout << "example: " << buffer.GetString() << end;

## Error: Assertion 'IsObject()' failed - rapidjson
This error occurs when you attempt to treat a member of a rapidjson::Value& variable as an object when it isn't.

### Array
One instance of this is when the root of a document is an Array, not an Object. If the array is of size one and contains an object, one must index into the array before attempting to utilize the inner object.

## Error: No CMAKE_CXX_Compiler could be found
This error can be solved by installing the gcc compiler. In a linux environment, this can be done by typing "apt-get install g++".

## Error: cannot retrieve consumer metadata with error: Local: Broker transport failure.
This error occurs when the PPM is unable to connect to the local broker. This can be caused by a number of things, including:
- The broker is not running
- The broker is using the wrong IP address
- The PPM is targeting the wrong IP address

# See Also: Testing/Troubleshooting
More information on troubleshooting can be found in the [Testing/Troubleshooting](../README.md#Testing/Troubleshooting) section of the README.