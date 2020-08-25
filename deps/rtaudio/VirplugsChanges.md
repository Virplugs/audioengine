# Changes made to RtAudio for the use in Virplugs

Base: v.5.1.0

- `RtAudio.cpp`: Added `long RtApi ::getStreamInputLatency(void) and `long RtApi ::getStreamOutputLatency(void)`.
- Added Windows implementation for `gettimeofday()`.
