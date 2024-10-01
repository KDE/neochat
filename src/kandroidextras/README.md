# KAndroidExtras

Utilities for using Java Native Interface (JNI) to interface with Android platform API.

## Java Native Interface (JNI) wrapper

C++ header-only code for defining compile-time checked JNI wrappers.

Supported:
- typed `jobject` wrappers (`Jni::Object`)
- wrappers for Java arrays holding primitive or non-primitive content (`Jni::Array`)
- reading static final properties (`JNI_CONSTANT`)
- reading and writing non-static properties (`JNI_PROPERTY`)
- static and non-static method calls, constructors (`JNI_METHOD`, `JNI_STATIC_METHOD`, `JNI_CONSTRUCTOR`)

Not yet supported:
- registering native methods for Java -> C++ calls

## JNI mock implementation

This is useful for automated testing of JNI code on other platforms than Android.

## Wrappers for Java and Android types

Predefined wrappers for common platform types needed in multiple places.
