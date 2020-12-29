
#define enum_to_string(x) #x

#ifdef DEBUG
#define debug(...) std::cout << __VA_ARGS__ << std::endl
#else
#define debug(...)
#endif
