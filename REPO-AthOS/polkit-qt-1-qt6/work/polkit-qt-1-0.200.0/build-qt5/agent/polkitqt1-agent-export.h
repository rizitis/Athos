
#ifndef POLKITQT1_AGENT_EXPORT_H
#define POLKITQT1_AGENT_EXPORT_H

#ifdef POLKITQT1_AGENT_STATIC_DEFINE
#  define POLKITQT1_AGENT_EXPORT
#  define POLKITQT1_AGENT_NO_EXPORT
#else
#  ifndef POLKITQT1_AGENT_EXPORT
#    ifdef polkit_qt5_agent_1_EXPORTS
        /* We are building this library */
#      define POLKITQT1_AGENT_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define POLKITQT1_AGENT_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef POLKITQT1_AGENT_NO_EXPORT
#    define POLKITQT1_AGENT_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef POLKITQT1_AGENT_DEPRECATED
#  define POLKITQT1_AGENT_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef POLKITQT1_AGENT_DEPRECATED_EXPORT
#  define POLKITQT1_AGENT_DEPRECATED_EXPORT POLKITQT1_AGENT_EXPORT POLKITQT1_AGENT_DEPRECATED
#endif

#ifndef POLKITQT1_AGENT_DEPRECATED_NO_EXPORT
#  define POLKITQT1_AGENT_DEPRECATED_NO_EXPORT POLKITQT1_AGENT_NO_EXPORT POLKITQT1_AGENT_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef POLKITQT1_AGENT_NO_DEPRECATED
#    define POLKITQT1_AGENT_NO_DEPRECATED
#  endif
#endif

#endif /* POLKITQT1_AGENT_EXPORT_H */
