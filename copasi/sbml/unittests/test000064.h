// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/sbml/unittests/test000064.h,v $
//   $Revision: 1.2 $
//   $Name:  $
//   $Author: gauges $
//   $Date: 2008/06/12 10:12:34 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#ifndef TEST_000064_H__
#define TEST_000064_H__

#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

class test000064 : public CppUnit::TestFixture
  {
    CPPUNIT_TEST_SUITE(test000064);
    CPPUNIT_TEST(test_import_rule_expression_and_hasOnlySubstanceUnits_1);
    CPPUNIT_TEST(test_import_rule_expression_and_hasOnlySubstanceUnits_2);
    CPPUNIT_TEST(test_import_rule_expression_and_hasOnlySubstanceUnits_3);
    CPPUNIT_TEST(test_import_rule_expression_and_hasOnlySubstanceUnits_4);
    CPPUNIT_TEST(test_import_rule_expression_and_hasOnlySubstanceUnits_5);
    CPPUNIT_TEST(test_import_rule_expression_and_hasOnlySubstanceUnits_6);
    CPPUNIT_TEST(test_import_rule_expression_and_hasOnlySubstanceUnits_7);
    CPPUNIT_TEST(test_import_rule_expression_and_hasOnlySubstanceUnits_8);
    CPPUNIT_TEST(test_import_event_assignment_expression_and_hasOnlySubstanceUnits_1);
    CPPUNIT_TEST(test_import_event_assignment_expression_and_hasOnlySubstanceUnits_2);
    CPPUNIT_TEST(test_import_event_assignment_expression_and_hasOnlySubstanceUnits_3);
    CPPUNIT_TEST(test_import_event_assignment_expression_and_hasOnlySubstanceUnits_4);
    CPPUNIT_TEST(test_import_event_assignment_expression_and_hasOnlySubstanceUnits_5);
    CPPUNIT_TEST(test_import_event_assignment_expression_and_hasOnlySubstanceUnits_6);
    CPPUNIT_TEST(test_import_event_assignment_expression_and_hasOnlySubstanceUnits_7);
    CPPUNIT_TEST(test_import_event_assignment_expression_and_hasOnlySubstanceUnits_8);
    CPPUNIT_TEST_SUITE_END();

  protected:
    // models for import test
    static const char* MODEL_STRING1;
    static const char* MODEL_STRING2;
    static const char* MODEL_STRING3;
    static const char* MODEL_STRING4;
    static const char* MODEL_STRING5;
    static const char* MODEL_STRING6;
    static const char* MODEL_STRING7;
    static const char* MODEL_STRING8;
    static const char* MODEL_STRING9;
    static const char* MODEL_STRING10;
    static const char* MODEL_STRING11;
    static const char* MODEL_STRING12;
    static const char* MODEL_STRING13;
    static const char* MODEL_STRING14;
    static const char* MODEL_STRING15;
    static const char* MODEL_STRING16;

    // models for export test
    static const char* MODEL_STRING101;
    static const char* MODEL_STRING102;
    static const char* MODEL_STRING103;
    static const char* MODEL_STRING104;

  public:
    void setUp();

    void tearDown();

    void test_import_rule_expression_and_hasOnlySubstanceUnits_1();
    void test_import_rule_expression_and_hasOnlySubstanceUnits_2();
    void test_import_rule_expression_and_hasOnlySubstanceUnits_3();
    void test_import_rule_expression_and_hasOnlySubstanceUnits_4();
    void test_import_rule_expression_and_hasOnlySubstanceUnits_5();
    void test_import_rule_expression_and_hasOnlySubstanceUnits_6();
    void test_import_rule_expression_and_hasOnlySubstanceUnits_7();
    void test_import_rule_expression_and_hasOnlySubstanceUnits_8();
    void test_import_event_assignment_expression_and_hasOnlySubstanceUnits_1();
    void test_import_event_assignment_expression_and_hasOnlySubstanceUnits_2();
    void test_import_event_assignment_expression_and_hasOnlySubstanceUnits_3();
    void test_import_event_assignment_expression_and_hasOnlySubstanceUnits_4();
    void test_import_event_assignment_expression_and_hasOnlySubstanceUnits_5();
    void test_import_event_assignment_expression_and_hasOnlySubstanceUnits_6();
    void test_import_event_assignment_expression_and_hasOnlySubstanceUnits_7();
    void test_import_event_assignment_expression_and_hasOnlySubstanceUnits_8();
  };

#endif /* TEST000064_H__ */
