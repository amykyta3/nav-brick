

/*
- Burst Mode

DIG_FILT
    Some sort of digital filter....
OSR/OSR2
    Magnetic/Temperature sensor ADC oversampling ratio
RES
    Resolution?
GAIN_SEL
    Analog chain gain setting, factor 5 between min and max code
    This appears to be on an inverse scale: Bigger GAIN_SEL value == less gain

    Z channel is slightly more sensitive than XY
    Based on the datasheet, selecting a GAIN_SEL +2 higher for Z roughly cancels it out
    Alternatively, set XY +1 RES, and Z -1 GAIN_SEL
    NOPE! GAIN_SEL is a global setting... RRGH

HALLCONF
    Hardly a good explanation is given, but it seems
    most things assume this is set to 0xC


Trading OSR vs DIG_FILT doesnt really matter.
In the end, a longer conversion time is best for RMS noise.
Shoot for ~200ms?

X/Y vs Z sensitivity is different. Fortunately it seems to be roughly consistent
Assuming same RES setting, normalize by:
    Z_normalized = Z_measured * 3300 / 2048

I should automatically select GAIN_SEL and/or RES based on the prior reading.
    Make up a single linear table of GAIN_SEL/RES settings rather than a 2d matrix
    If maximum of X/Y/Z is greater than 0.75 of full-scale, then decrement gain/res
    If maximum of X/Y/Z is less than 0.25 of full-scale, then increment gain/res
    .. Or something like that..
    Basically make a feedback loop that aims for 50% or 75% of full scale such that
    I maximize precision on translation to polar coordinates

    No need to actually know the true precision numbers of any of these combos,
    since I dont actually care about the magnitude of the vector. Just the direction.


*/
