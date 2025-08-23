#pragma once
#include "common/base.hpp"
#include "demo/export.hpp"

namespace demo {

#ifndef DEMO_EXPORT_ON_TEMPLATE
#define DEMO_EXPORT_ON_TEMPLATE 0
#endif
#ifndef DEMO_EXPORT_ON_EXTERN
#define DEMO_EXPORT_ON_EXTERN 0
#endif
#ifndef DEMO_EXPORT_ON_INSTANTIATION
#define DEMO_EXPORT_ON_INSTANTIATION 0
#endif

#if DEMO_EXPORT_ON_TEMPLATE
  #define DEMO_TMPL_EXPORT DEMO_EXPORT
#else
  #define DEMO_TMPL_EXPORT
#endif

template <class T>
class DEMO_TMPL_EXPORT Vec : public common::Base {
public:
  Vec() : data_{} {}
  explicit Vec(T v) : data_{v} {}
#ifndef DEMO_INLINE_DTOR
#define DEMO_INLINE_DTOR 0
#endif
#if DEMO_INLINE_DTOR
  virtual ~Vec() = default;
#else
  virtual ~Vec(); // key function out-of-line
#endif
  virtual T get() const { return data_; }
  void set(T v) { data_ = v; }
private:
  T data_;
};

#ifdef DEMO_DO_INSTANTIATE
  #define DEMO_EXTERN_KEY
  #if DEMO_EXPORT_ON_INSTANTIATION
    #define DEMO_LINE_EXPORT DEMO_EXPORT
  #else
    #define DEMO_LINE_EXPORT
  #endif
#else
  #define DEMO_EXTERN_KEY extern
  #if DEMO_EXPORT_ON_EXTERN
    #define DEMO_LINE_EXPORT DEMO_EXPORT
  #else
    #define DEMO_LINE_EXPORT
  #endif
#endif

DEMO_EXTERN_KEY template class DEMO_LINE_EXPORT Vec<int>;
DEMO_EXTERN_KEY template class DEMO_LINE_EXPORT Vec<double>;

#undef DEMO_TMPL_EXPORT
#undef DEMO_EXTERN_KEY
#undef DEMO_LINE_EXPORT

} // namespace demo
