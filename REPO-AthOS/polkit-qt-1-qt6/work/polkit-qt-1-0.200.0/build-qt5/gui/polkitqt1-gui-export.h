
#ifndef POLKITQT1_GUI_EXPORT_H
#define POLKITQT1_GUI_EXPORT_H

#ifdef POLKITQT1_GUI_STATIC_DEFINE
#  define POLKITQT1_GUI_EXPORT
#  define POLKITQT1_GUI_NO_EXPORT
#else
#  ifndef POLKITQT1_GUI_EXPORT
#    ifdef MAKE_POLKITQT1_LIB
        /* We are building this library */
#      define POLKITQT1_GUI_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define POLKITQT1_GUI_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef POLKITQT1_GUI_NO_EXPORT
#    define POLKITQT1_GUI_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef POLKITQT1_GUI_DEPRECATED
#  define POLKITQT1_GUI_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef POLKITQT1_GUI_DEPRECATED_EXPORT
#  define POLKITQT1_GUI_DEPRECATED_EXPORT POLKITQT1_GUI_EXPORT POLKITQT1_GUI_DEPRECATED
#endif

#ifndef POLKITQT1_GUI_DEPRECATED_NO_EXPORT
#  define POLKITQT1_GUI_DEPRECATED_NO_EXPORT POLKITQT1_GUI_NO_EXPORT POLKITQT1_GUI_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef POLKITQT1_GUI_NO_DEPRECATED
#    define POLKITQT1_GUI_NO_DEPRECATED
#  endif
#endif

#endif /* POLKITQT1_GUI_EXPORT_H */
