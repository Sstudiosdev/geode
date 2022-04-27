#pragma once
#include "Wrapper.hpp"
#include "Types.hpp"
#include "Addresses.hpp"
#include "../meta/meta.hpp"
#include "../loader/Interface.hpp"
#include <iostream>

#define GEODE_APPLY_MODIFY_FOR_FUNCTION(index, convention, className, functionName)                                                          \
using base##index = wrap::functionName<Base, types::pure##index>;                                                            \
using derived##index = wrap::functionName<Derived, types::pure##index>;                                                      \
if constexpr (derived##index::uuid != nullptr && (void*)base##index::uuid != (void*)derived##index::uuid) {                                  \
	Interface::get()->logInfo(                                                                                                               \
		"Adding hook at function " #className "::" #functionName,                                                                            \
		Severity::Debug                                                                                                                      \
	);                                                                                                                                       \
	Interface::get()->addHook<derived##index::value, convention>(                                                                            \
		#className "::" #functionName,                                                                                                       \
		(void*)addresses::address##index()                                                                                                   \
	);                                                                                                                                       \
}                                                                                                                                            \

namespace geode::modifier {

	template <class Derived, class Base>
	class Modify;

	template <class Derived>
	class ModifyBase {
		// unordered_map<handles> idea
		ModifyBase() {
			Derived::apply();
		}
		template <class, class>
		friend class Modify;
		// explicit Modify(Property property) idea
	};

	template <class Derived, class Base>
	class Modify {
		Modify() {
			static_assert(std::is_same_v<Derived, Derived>, "Custom Modify not implemented.");
		}
	};

	#include <gen/GeneratedModify.hpp>
}
