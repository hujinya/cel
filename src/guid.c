#include "cel/_guid.h"

BOOL IsLowerHexDigit(char c) 
{
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

BOOL IsValidGUIDInternal(const char *guid, BOOL strict) 
{
  //const size_t kGUIDLength = 36U;
  //if (guid.length() != kGUIDLength)
  //  return FALSE;

  //for (size_t i = 0; i < guid.length(); ++i) 
  //{
  //  char current = guid[i];
  //  if (i == 8 || i == 13 || i == 18 || i == 23) 
  //  {
  //    if (current != '-')
  //      return FALSE;
  //  } 
  //  else 
  //  {
  //    if ((strict && !IsLowerHexDigit(current)) || !IsHexDigit(current))
  //      return FALSE;
  //  }
  //}

  return TRUE;
}

const char *cel_guid_generate() 
{
  //U64 sixteen_bytes[2] = {base::cel_rand_u64(), base::cel_rand_u64()};

  //// Set the GUID to version 4 as described in RFC 4122, section 4.4.
  //// The format of GUID version 4 must be xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx,
  //// where y is one of [8, 9, A, B].

  //// Clear the version bits and set the version to 4:
  //sixteen_bytes[0] &= 0xffffffffffff0fffULL;
  //sixteen_bytes[0] |= 0x0000000000004000ULL;

  //// Set the two most significant bits (bits 6 and 7) of the
  //// clock_seq_hi_and_reserved to zero and one, respectively:
  //sixteen_bytes[1] &= 0x3fffffffffffffffULL;
  //sixteen_bytes[1] |= 0x8000000000000000ULL;

  //return RandomDataToGUIDString(sixteen_bytes);
    return NULL;
}

//BOOL IsValidGUID(const base::StringPiece& guid) 
//{
//  return IsValidGUIDInternal(guid, FALSE /* strict */);
//}
//
//BOOL IsValidGUIDOutputString(const base::StringPiece& guid) 
//{
//  return IsValidGUIDInternal(guid, TRUE /* strict */);
//}
//
//char * RandomDataToGUIDString(const U64 bytes[2]) 
//{
//  return StringPrintf("%08x-%04x-%04x-%04x-%012llx",
//                      static_cast<unsigned int>(bytes[0] >> 32),
//                      static_cast<unsigned int>((bytes[0] >> 16) & 0x0000ffff),
//                      static_cast<unsigned int>(bytes[0] & 0x0000ffff),
//                      static_cast<unsigned int>(bytes[1] >> 48),
//                      bytes[1] & 0x0000ffffffffffffULL);
//}
