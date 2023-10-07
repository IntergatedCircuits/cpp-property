// SPDX-License-Identifier: MIT
#include <cstddef>
#include <type_traits>

template<typename TProp, typename TValue, typename TOwner,
    TValue (TOwner::*TGetter)() const, void (TOwner::*TSetter)(TValue)>
class property_base
{
protected:
    constexpr property_base() = default;
    property_base(const property_base& other) = delete;

    TOwner& owner()
    {
        static_assert(sizeof(*this) == 1);
#ifdef _MSC_VER
        static_assert(!std::is_polymorphic_v<TOwner>);
#endif
        return reinterpret_cast<TOwner&>(*(this + derived().owner_offset()));
    }
    const TOwner& owner() const
    {
        static_assert(sizeof(*this) == 1);
#ifdef _MSC_VER
        static_assert(!std::is_polymorphic_v<TOwner>);
#endif
        return reinterpret_cast<const TOwner&>(*(this + derived().owner_offset()));
    }
    static constexpr bool gettable() { return TGetter != nullptr; }
    TValue get() const requires(gettable()) { return (owner().*TGetter)(); }

    static constexpr bool settable() { return TSetter != nullptr; }
    void set(TValue v) requires(settable()) { (owner().*TSetter)(v); }

private: // CRTP
    TProp& derived() { return static_cast<TProp&>(*this); }
    const TProp& derived() const { return static_cast<const TProp&>(*this); }
};

/// @brief This macro creates a property field of a class with a given getter and setter
/// member function.
/// @param TYPE: the data type of the property
/// @param NAME: the name of the property member variable
/// @param CLASS: the name of the property owning class
/// @param GET_ACCESS: get access level of the property, either public or private
/// @param GET_FN: name of the property getter member function
/// @param SET_ACCESS: set access level of the property, either public or private
/// @param SET_FN: name of the property setter member function
#define PROPERTY(TYPE, NAME, CLASS, GET_ACCESS, GET_FN, SET_ACCESS, SET_FN)                  \
  class _##NAME##_type                                                                       \
      : property_base<_##NAME##_type, TYPE, CLASS, &CLASS::GET_FN, &CLASS::SET_FN>           \
  {                                                                                          \
  public:                                                                                    \
    using value_type = TYPE;                                                                 \
  private:                                                                                   \
    friend CLASS;                                                                            \
    using base = property_base<_##NAME##_type, TYPE, CLASS, &CLASS::GET_FN, &CLASS::SET_FN>; \
    friend base;                                                                             \
    static inline auto owner_offset() { return offsetof(CLASS, NAME); }                      \
    constexpr _##NAME##_type() = default;                                                    \
  GET_ACCESS:                                                                                \
    using base::get;                                                                         \
    operator value_type() const { return get(); }                                            \
  SET_ACCESS:                                                                                \
    using base::set;                                                                         \
    _##NAME##_type& operator=(value_type v) { set(v); return *this; }                        \
    _##NAME##_type& operator=(const _##NAME##_type& other)                                   \
    { set(other.get()); return *this; }                                                      \
  };                                                                                         \
  [[no_unique_address]] _##NAME##_type NAME

/// @brief This macro creates a read-only property field of a class with a given getter
/// member function.
/// @param TYPE: the data type of the property
/// @param NAME: the name of the property member variable
/// @param CLASS: the name of the property owning class
/// @param GET_ACCESS: get access level of the property, either public or private
/// @param GET_FN: name of the property getter member function
#define PROPERTY_RO(TYPE, NAME, CLASS, GET_ACCESS, GET_FN)                                   \
  class _##NAME##_type                                                                       \
      : property_base<_##NAME##_type, TYPE, CLASS, &CLASS::GET_FN, nullptr>                  \
  {                                                                                          \
  public:                                                                                    \
    using value_type = TYPE;                                                                 \
  private:                                                                                   \
    friend CLASS;                                                                            \
    using base = property_base<_##NAME##_type, TYPE, CLASS, &CLASS::GET_FN, nullptr>;        \
    friend base;                                                                             \
    static inline auto owner_offset() { return offsetof(CLASS, NAME); }                      \
    constexpr _##NAME##_type() = default;                                                    \
  GET_ACCESS:                                                                                \
    using base::get;                                                                         \
    operator value_type() const { return get(); }                                            \
  };                                                                                         \
  [[no_unique_address]] _##NAME##_type NAME

/// @brief This macro creates a write-only property field of a class with a given setter
/// member function.
/// @param TYPE: the data type of the property
/// @param NAME: the name of the property member variable
/// @param CLASS: the name of the property owning class
/// @param GET_ACCESS: get access level of the property, either public or private
/// @param GET_FN: name of the property getter member function
#define PROPERTY_WO(TYPE, NAME, CLASS, SET_ACCESS, SET_FN)                                   \
  class _##NAME##_type                                                                       \
      : property_base<_##NAME##_type, TYPE, CLASS, nullptr, &CLASS::SET_FN>                  \
  {                                                                                          \
  public:                                                                                    \
    using value_type = TYPE;                                                                 \
  private:                                                                                   \
    friend CLASS;                                                                            \
    using base = property_base<_##NAME##_type, TYPE, CLASS, nullptr, &CLASS::SET_FN>;        \
    friend base;                                                                             \
    static inline auto owner_offset() { return offsetof(CLASS, NAME); }                      \
    constexpr _##NAME##_type() = default;                                                    \
  SET_ACCESS:                                                                                \
    using base::set;                                                                         \
    _##NAME##_type& operator=(value_type v) { set(v); return *this; }                        \
  };                                                                                         \
  [[no_unique_address]] _##NAME##_type NAME
