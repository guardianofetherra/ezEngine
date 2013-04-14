#include <PCH.h>

EZ_CREATE_SIMPLE_TEST(Basics, ArrayPtr)
{
  EZ_TEST_BLOCK(true, "Empty Constructor")
  {
    ezArrayPtr<ezInt32> Empty;

    EZ_TEST(Empty.GetPtr() == NULL);
    EZ_TEST(Empty.GetCount() == 0);
  }

  EZ_TEST_BLOCK(true, "Constructor")
  {
    ezInt32 pIntData[] = { 1, 2, 3, 4, 5 };

    ezArrayPtr<ezInt32> ap(pIntData, 3);
    EZ_TEST(ap.GetPtr() == pIntData);
    EZ_TEST(ap.GetCount() == 3);

    ezArrayPtr<ezInt32> ap2(pIntData, 0);
    EZ_TEST(ap2.GetPtr() == NULL);
    EZ_TEST(ap2.GetCount() == 0);

    ezArrayPtr<ezInt32> ap3(pIntData);
    EZ_TEST(ap3.GetPtr() == pIntData);
    EZ_TEST(ap3.GetCount() == 5);

    ezArrayPtr<ezInt32> ap4(ap);
    EZ_TEST(ap4.GetPtr() == pIntData);
    EZ_TEST(ap4.GetCount() == 3);
  }

  EZ_TEST_BLOCK(true, "operator=")
  {
    ezInt32 pIntData[] = { 1, 2, 3, 4, 5 };

    ezArrayPtr<ezInt32> ap(pIntData, 3);
    EZ_TEST(ap.GetPtr() == pIntData);
    EZ_TEST(ap.GetCount() == 3);

    ezArrayPtr<ezInt32> ap2;
    ap2 = ap;

    EZ_TEST(ap2.GetPtr() == pIntData);
    EZ_TEST(ap2.GetCount() == 3);
  }

  EZ_TEST_BLOCK(true, "Reset")
  {
    ezInt32 pIntData[] = { 1, 2, 3, 4, 5 };

    ezArrayPtr<ezInt32> ap(pIntData, 3);
    EZ_TEST(ap.GetPtr() == pIntData);
    EZ_TEST(ap.GetCount() == 3);

    ap.Reset();

    EZ_TEST(ap.GetPtr() == NULL);
    EZ_TEST(ap.GetCount() == 0);
  }

  EZ_TEST_BLOCK(true, "operator== / operator!=")
  {
    ezInt32 pIntData[] = { 1, 2, 3, 4, 5 };

    ezArrayPtr<ezInt32> ap1(pIntData, 3);
    ezArrayPtr<ezInt32> ap2(pIntData, 3);
    ezArrayPtr<ezInt32> ap3(pIntData, 4);
    ezArrayPtr<ezInt32> ap4(pIntData + 1, 3);

    EZ_TEST(ap1 == ap2);
    EZ_TEST(ap1 != ap3);
    EZ_TEST(ap1 != ap4);
  }

  EZ_TEST_BLOCK(true, "operator[]")
  {
    ezInt32 pIntData[] = { 1, 2, 3, 4, 5 };

    ezArrayPtr<ezInt32> ap(pIntData + 1, 3);
    EZ_TEST_INT(ap[0], 2);
    EZ_TEST_INT(ap[1], 3);
    EZ_TEST_INT(ap[2], 4);
  }

  EZ_TEST_BLOCK(true, "const operator[]")
  {
    ezInt32 pIntData[] = { 1, 2, 3, 4, 5 };

    const ezArrayPtr<ezInt32> ap(pIntData + 1, 3);
    EZ_TEST_INT(ap[0], 2);
    EZ_TEST_INT(ap[1], 3);
    EZ_TEST_INT(ap[2], 4);
  }

  EZ_TEST_BLOCK(true, "CopyFrom")
  {
    ezInt32 pIntData1[] = { 1, 2, 3, 4, 5 };
    ezInt32 pIntData2[] = { 6, 7, 8, 9, 0 };

    ezArrayPtr<ezInt32> ap1(pIntData1 + 1, 3);
    ezArrayPtr<ezInt32> ap2(pIntData2 + 2, 3);

    ap1.CopyFrom(ap2);

    EZ_TEST_INT(pIntData1[0], 1);
    EZ_TEST_INT(pIntData1[1], 8);
    EZ_TEST_INT(pIntData1[2], 9);
    EZ_TEST_INT(pIntData1[3], 0);
    EZ_TEST_INT(pIntData1[4], 5);
  }

  EZ_TEST_BLOCK(true, "GetSubArray")
  {
    ezInt32 pIntData1[] = { 1, 2, 3, 4, 5 };

    ezArrayPtr<ezInt32> ap1(pIntData1, 5);
    ezArrayPtr<ezInt32> ap2 = ap1.GetSubArray(2, 3);

    EZ_TEST(ap2.GetPtr() == &pIntData1[2]);
    EZ_TEST(ap2.GetCount() == 3);
  }
}
