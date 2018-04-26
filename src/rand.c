#include "cel/_rand.h"

U64 cel_rand_generator(U64 range) 
{
    // We must discard random results above this number, as they would
    // make the random generator non-uniform (consider e.g. if
    // MAX_UINT64 was 7 and |range| was 5, then a result of 1 would be twice
    // as likely as a result of 3 or 4).
    //U64 max_acceptable_value =
    //    (std::numeric_limits<U64>::max() / range) * range - 1;

    //U64 value;
    //do 
    //{
    //    value = cel_rand_u64();
    //} while (value > max_acceptable_value);

    //return value % range;
    return 0;
}

int cel_rand_int(int min, int max) 
{
    U64 range = (U64)(max) - min + 1;
    // |range| is at most UINT_MAX + 1, so the result of cel_rand_generator(range)
    // is at most UINT_MAX.  Hence it's safe to cast it from U64 to int64_t.
    int result = (int)(min + cel_rand_generator(range));

    return result;
}

double cel_rand_double_interval(U64 bits) 
{
    // We try to get maximum precision by masking out as many bits as will fit
    // in the target type's mantissa, and raising it to an appropriate power to
    // produce output in the range [0, 1).  For IEEE 754 doubles, the mantissa
    // is expected to accommodate 53 bits.

    //static_assert(std::numeric_limits<double>::radix == 2,
    //    "otherwise use scalbn");
    //static const int kBits = std::numeric_limits<double>::digits;
    //U64 random_bits = bits & ((UINT64_C(1) << kBits) - 1);
    //double result = ldexp(static_cast<double>(random_bits), -1 * kBits);

    //return result;
    return 0;
}

double cel_rand_double() 
{
    return 0;
        //cel_rand_double_interval(cel_rand_u64());
}

char *cel_rand_string(size_t length) 
{
    //DCHECK_GT(length, 0u);
    //std::string result;
    //RandBytes(WriteInto(&result, length + 1), length);
    //return result;
    return NULL;
}