#ifndef KADR_VERSION_H
#define KADR_VERSION_H

#if !defined(VERSION_TAG)
#define VERSION_TAG "0.0.0"
#endif

#if !defined(GIT_HASH)
#define GIT_HASH "0"
#endif

#if !defined(GIT_COMMITS)
#define GIT_COMMITS "0"
#endif

#if !defined(BUILD_NUMBER)
#define BUILD_NUMBER "0"
#endif

#if !defined(GIT_BRANCH)
#define GIT_BRANCH "0"
#endif

#if !defined(GIT_DIRTY)
#define GIT_DIRTY 0
#endif

#define VERSION_FULL VERSION_TAG "+" GIT_COMMITS "." GIT_HASH "+" BUILD_NUMBER
#define VERSION_SHORT VERSION_TAG

#if GIT_DIRTY
#define VERSION_FULL_DIRTY VERSION_FULL "-dirty"
#else
#define VERSION_FULL_DIRTY VERSION_FULL
#endif

#endif  //KADR_VERSION_H
