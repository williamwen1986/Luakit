// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/test/gtest_xml_util.h"

#include "base/file_util.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/test/launcher/test_launcher.h"
#include "third_party/libxml/chromium/libxml_utils.h"

namespace base {

namespace {

// This is used for the xml parser to report errors. This assumes the context
// is a pointer to a std::string where the error message should be appended.
static void XmlErrorFunc(void *context, const char *message, ...) {
  va_list args;
  va_start(args, message);
  std::string* error = static_cast<std::string*>(context);
  base::StringAppendV(error, message, args);
  va_end(args);
}

}  // namespace

XmlUnitTestResultPrinter::XmlUnitTestResultPrinter() : output_file_(NULL) {
}

XmlUnitTestResultPrinter::~XmlUnitTestResultPrinter() {
  if (output_file_) {
    fprintf(output_file_, "</testsuites>\n");
    fflush(output_file_);
    base::CloseFile(output_file_);
  }
}

bool XmlUnitTestResultPrinter::Initialize(const FilePath& output_file_path) {
  DCHECK(!output_file_);
  output_file_ = OpenFile(output_file_path, "w");
  if (!output_file_)
    return false;

  fprintf(output_file_,
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<testsuites>\n");
  fflush(output_file_);

  return true;
}

void XmlUnitTestResultPrinter::OnTestCaseStart(
    const testing::TestCase& test_case) {
  fprintf(output_file_, "  <testsuite>\n");
  fflush(output_file_);
}

void XmlUnitTestResultPrinter::OnTestStart(const testing::TestInfo& test_info) {
  // This is our custom extension - it helps to recognize which test was running
  // when the test binary crashed. Note that we cannot even open the <testcase>
  // tag here - it requires e.g. run time of the test to be known.
  fprintf(output_file_,
          "    <x-teststart name=\"%s\" classname=\"%s\" />\n",
          test_info.name(),
          test_info.test_case_name());
  fflush(output_file_);
}

void XmlUnitTestResultPrinter::OnTestEnd(const testing::TestInfo& test_info) {
  fprintf(output_file_,
          "    <testcase name=\"%s\" status=\"run\" time=\"%.3f\""
          " classname=\"%s\">\n",
          test_info.name(),
          static_cast<double>(test_info.result()->elapsed_time()) /
              Time::kMillisecondsPerSecond,
          test_info.test_case_name());
  if (test_info.result()->Failed())
    fprintf(output_file_, "      <failure message=\"\" type=\"\"></failure>\n");
  fprintf(output_file_, "    </testcase>\n");
  fflush(output_file_);
}

void XmlUnitTestResultPrinter::OnTestCaseEnd(
    const testing::TestCase& test_case) {
  fprintf(output_file_, "  </testsuite>\n");
  fflush(output_file_);
}

bool ProcessGTestOutput(const base::FilePath& output_file,
                        std::vector<TestResult>* results,
                        bool* crashed) {
  DCHECK(results);

  std::string xml_contents;
  if (!ReadFileToString(output_file, &xml_contents))
    return false;

  // Silence XML errors - otherwise they go to stderr.
  std::string xml_errors;
  ScopedXmlErrorFunc error_func(&xml_errors, &XmlErrorFunc);

  XmlReader xml_reader;
  if (!xml_reader.Load(xml_contents))
    return false;

  enum {
    STATE_INIT,
    STATE_TESTSUITE,
    STATE_TESTCASE,
    STATE_FAILURE,
    STATE_END,
  } state = STATE_INIT;

  while (xml_reader.Read()) {
    xml_reader.SkipToElement();
    std::string node_name(xml_reader.NodeName());

    switch (state) {
      case STATE_INIT:
        if (node_name == "testsuites" && !xml_reader.IsClosingElement())
          state = STATE_TESTSUITE;
        else
          return false;
        break;
      case STATE_TESTSUITE:
        if (node_name == "testsuites" && xml_reader.IsClosingElement())
          state = STATE_END;
        else if (node_name == "testsuite" && !xml_reader.IsClosingElement())
          state = STATE_TESTCASE;
        else
          return false;
        break;
      case STATE_TESTCASE:
        if (node_name == "testsuite" && xml_reader.IsClosingElement()) {
          state = STATE_TESTSUITE;
        } else if (node_name == "x-teststart" &&
                   !xml_reader.IsClosingElement()) {
          // This is our custom extension that helps recognize which test was
          // running when the test binary crashed.
          TestResult result;

          std::string test_case_name;
          if (!xml_reader.NodeAttribute("classname", &test_case_name))
            return false;
          std::string test_name;
          if (!xml_reader.NodeAttribute("name", &test_name))
            return false;
          result.full_name = test_case_name + "." + test_name;

          result.elapsed_time = TimeDelta();

          // Assume the test crashed - we can correct that later.
          result.status = TestResult::TEST_CRASH;

          results->push_back(result);
        } else if (node_name == "testcase" && !xml_reader.IsClosingElement()) {
          std::string test_status;
          if (!xml_reader.NodeAttribute("status", &test_status))
            return false;

          if (test_status != "run" && test_status != "notrun")
            return false;
          if (test_status != "run")
            break;

          TestResult result;

          std::string test_case_name;
          if (!xml_reader.NodeAttribute("classname", &test_case_name))
            return false;
          std::string test_name;
          if (!xml_reader.NodeAttribute("name", &test_name))
            return false;
          result.full_name = test_case_name + "." + test_name;

          std::string test_time_str;
          if (!xml_reader.NodeAttribute("time", &test_time_str))
            return false;
          result.elapsed_time =
              TimeDelta::FromMicroseconds(strtod(test_time_str.c_str(), NULL) *
                                          Time::kMicrosecondsPerSecond);

          result.status = TestResult::TEST_SUCCESS;

          if (!results->empty() &&
              results->at(results->size() - 1).full_name == result.full_name &&
              results->at(results->size() - 1).status ==
                  TestResult::TEST_CRASH) {
            // Erase the fail-safe "crashed" result - now we know the test did
            // not crash.
            results->pop_back();
          }

          results->push_back(result);
        } else if (node_name == "failure" && !xml_reader.IsClosingElement()) {
          std::string failure_message;
          if (!xml_reader.NodeAttribute("message", &failure_message))
            return false;

          DCHECK(!results->empty());
          results->at(results->size() - 1).status = TestResult::TEST_FAILURE;

          state = STATE_FAILURE;
        } else if (node_name == "testcase" && xml_reader.IsClosingElement()) {
          // Deliberately empty.
        } else {
          return false;
        }
        break;
      case STATE_FAILURE:
        if (node_name == "failure" && xml_reader.IsClosingElement())
          state = STATE_TESTCASE;
        else
          return false;
        break;
      case STATE_END:
        // If we are here and there are still XML elements, the file has wrong
        // format.
        return false;
    }
  }

  *crashed = (state != STATE_END);
  return true;
}

}  // namespace base
