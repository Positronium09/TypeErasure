export module TypeErasure:Erase;

import std;

namespace TypeErasure::Detail
{
	struct DummyTemplateCheck { };

	template <typename T, template <typename...> typename Template>
	struct IsSpecializationHelper : std::false_type { };

	template <template <typename...> typename Template, typename... Args>
	struct IsSpecializationHelper<Template<Args...>, Template> : std::true_type { };

	template <typename, typename = void>
	struct HasTemplatedVTable : std::false_type { };
	template <typename T>
	struct HasTemplatedVTable<T, std::void_t<typename T::template VTable<DummyTemplateCheck>>> : std::true_type { };
	template <typename, typename = void>
	struct HasTemplatedModel : std::false_type { };
	template <typename T>
	struct HasTemplatedModel<T, std::void_t<typename T::template Model<DummyTemplateCheck>>> : std::true_type { };
	template <typename, typename = void>
	struct HasTemplatedInterface : std::false_type { };
	template <typename T>
	struct HasTemplatedInterface<T, std::void_t<typename T::template Interface<DummyTemplateCheck>>> : std::true_type { };
	template <typename, typename = void>
	struct HasValidator : std::false_type { };
	template <typename T>
	struct HasValidator<T,
	std::void_t<decltype(T::template Validator<DummyTemplateCheck>::value)>> : std::true_type { };

	template <typename, typename U>
	struct ValidateHelper : std::true_type
	{
	};
	template <typename T, typename U> requires HasValidator<U>::value
	struct ValidateHelper<T, U>
	{
		static constexpr bool value = U::template Validator<T>::value;
	};
}

export namespace TypeErasure
{
	template <typename T, template <typename...> typename Template>
	concept IsSpecialization = Detail::IsSpecializationHelper<T, Template>::value;

	template <typename T>
	concept Validated = Detail::HasValidator<T>::value;

	template <typename T, typename... Types>
	concept InParameterPack = (std::is_same_v<T, Types> || ...);

	template <typename T>
	concept FeatureType =
		Detail::HasTemplatedVTable<T>::value &&
		Detail::HasTemplatedModel<T>::value &&
		Detail::HasTemplatedInterface<T>::value;

	template <typename T, typename... Features>
	concept ValidateType = (Detail::ValidateHelper<T, Features>::value && ...) && (FeatureType<Features> && ...);
	template <typename T, typename... Features>
	concept SupportsFeatures = ValidateType<T, Features...>;
	template <typename T, typename Feature>
	concept SupportsFeature = ValidateType<T, Feature>;

	struct VTableBase
	{
		virtual ~VTableBase() noexcept = default;

		virtual auto Type() const noexcept -> const std::type_info& = 0;

		virtual constexpr auto IsRef() const noexcept -> bool = 0;
		virtual constexpr auto IsCRef() const noexcept -> bool = 0;
		virtual constexpr auto GetRawObjectPtr() const noexcept -> const void* = 0;
		virtual constexpr auto GetRawObjectPtr() noexcept -> void* = 0;

		template <typename T>
		constexpr auto GetObjectPtr() noexcept -> T*
		{
			if (Type() != typeid(T) && Type() != typeid(std::remove_cv_t<T>))
			{
				return nullptr;
			}
			return static_cast<T*>(GetRawObjectPtr());
		}
		template <typename T>
		constexpr auto GetObjectPtr() const noexcept -> const T*
		{
			if (Type() != typeid(T) && Type() != typeid(std::remove_cv_t<T>))
			{
				return nullptr;
			}
			return static_cast<const T*>(GetRawObjectPtr());
		}

		template <typename T>
		constexpr auto GetObjectPtrCheckless() noexcept -> T*
		{
			return static_cast<T*>(GetRawObjectPtr());
		}
		template <typename T>
		constexpr auto GetObjectPtrCheckless() const noexcept -> const T*
		{
			return static_cast<const T*>(GetRawObjectPtr());
		}
	};

	template <typename Base, FeatureType... Features>
	struct VtableComposer : virtual Base, Features::template VTable<Base>...
	{
		using Type = VtableComposer;
	};

	template <typename Base, FeatureType... Features>
	struct ModelComposer;

	template <typename Base>
	struct ModelComposer<Base>
	{
		using Type = Base;
	};

	template <typename Base, FeatureType Feature, FeatureType... Others>
	struct ModelComposer<Base, Feature, Others...>
	{
		using Composed = Feature::template Model<Base>;
		using Type = ModelComposer<Composed, Others...>::Type;
	};

	template <typename T, typename VTable>
	class ModelBase : public VTable
	{
		public:
		using ObjectType = T;

		template <typename... Args>
		explicit ModelBase(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) :
			object{ std::forward<Args>(args)... }
		{
		}

		explicit ModelBase(const T& obj) noexcept(std::is_nothrow_copy_constructible_v<T>) :
			object{ obj }
		{
		}

		auto& GetObject() noexcept
		{
			return object;
		}

		const auto& GetObject() const noexcept
		{
			return object;
		}

		auto Type() const noexcept -> const std::type_info& override
		{
			return typeid(T);
		}

		constexpr auto IsRef() const noexcept -> bool override
		{
			return false;
		}
		constexpr auto IsCRef() const noexcept -> bool override
		{
			return false;
		}

		constexpr auto GetRawObjectPtr() const noexcept -> const void* override
		{
			return std::addressof(object);
		}
		constexpr auto GetRawObjectPtr() noexcept -> void* override
		{
			return std::addressof(object);
		}

		private:
		T object;
	};

	template <typename T, typename VTable>
	class ModelBase<T&, VTable> : public VTable
	{
		public:
		using ObjectType = T;

		explicit ModelBase(T& obj) noexcept :
			object{ obj }
		{
		}

		auto& GetObject() noexcept
		{
			return object.get();
		}

		const auto& GetObject() const noexcept
		{
			return object.get();
		}

		auto Type() const noexcept -> const std::type_info & override
		{
			return typeid(T);
		}

		constexpr auto IsRef() const noexcept -> bool override
		{
			return true;
		}
		constexpr auto IsCRef() const noexcept -> bool override
		{
			return false;
		}

		constexpr auto GetRawObjectPtr() const noexcept -> const void* override
		{
			return std::addressof(object.get());
		}
		constexpr auto GetRawObjectPtr() noexcept -> void* override
		{
			return std::addressof(object.get());
		}

		private:
		std::reference_wrapper<T> object;
	};

	template <typename T, typename VTable>
	class ModelBase<const T&, VTable> : public VTable
	{
		public:
		using ObjectType = T;

		explicit ModelBase(const T& obj) noexcept :
			object{ obj }
		{
		}

		const auto& GetObject() const noexcept
		{
			return object.get();
		}

		auto Type() const noexcept -> const std::type_info& override
		{
			return typeid(T);
		}

		constexpr auto IsRef() const noexcept -> bool override
		{
			return true;
		}
		constexpr auto IsCRef() const noexcept -> bool override
		{
			return true;
		}

		constexpr auto GetRawObjectPtr() const noexcept -> const void* override
		{
			return std::addressof(object);
		}
		constexpr auto GetRawObjectPtr() noexcept -> void* override
		{
			return std::addressof(object);
		}

		private:
		std::reference_wrapper<const T> object;
	};

	template <typename T, FeatureType... Features>
	struct CompleteModel : ModelComposer<
		ModelBase<T, typename VtableComposer<VTableBase, Features...>::Type>,
		Features...>::Type
	{
		using Base = ModelComposer<
			ModelBase<T, typename VtableComposer<VTableBase, Features...>::Type>,
			Features...>::Type;

		template <typename U>
		explicit CompleteModel(U&& obj) noexcept(std::is_nothrow_constructible_v<T, U>) :
			Base{ std::forward<U>(obj) }
		{
		}
	};
	
	template <typename Base, FeatureType... Features>
	struct InterfaceComposer;

	template <typename Base>
	struct InterfaceComposer<Base>
	{
		using Type = Base;
	};

	template <typename Base, FeatureType Feature, FeatureType... Others>
	struct InterfaceComposer<Base, Feature, Others...>
	{
		using Composed = Feature::template Interface<Base>;
		using Type = InterfaceComposer<Composed, Others...>::Type;
	};

	struct AnyBase
	{
		auto GetVTable() const noexcept
		{
			return vtable.get();
		}

		auto HasValue() const noexcept
		{
			return vtable != nullptr;
		}

		auto Reset() noexcept -> void
		{
			if (!HasValue())
			{
				return;
			}
			vtable.reset();
		}

		const auto& Type() const noexcept
		{
			if (!HasValue())
			{
				return typeid(void);
			}
			return vtable->Type();
		}

		protected:
		std::shared_ptr<VTableBase> vtable{ nullptr };
	};

	struct AnyTypeInformation
	{
		const std::type_info& type;
		bool isRef;
		bool isCRef;
	};

	template <FeatureType... Features>
	struct Any : InterfaceComposer<AnyBase, Features...>::Type
	{
		Any() = default;

		~Any() noexcept
		{
			this->Reset();
		}
		template <typename T> requires !IsSpecialization<T, Any> && ValidateType<T, Features...>
		explicit Any(T&& obj) noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T>)
		{
			using Decayed = std::decay_t<T>;
			using Model = CompleteModel<Decayed, Features...>;
			this->vtable = std::make_shared<Model>(std::forward<T>(obj));
		}
		template <typename T> requires ValidateType<T, Features...>
		explicit Any(T& obj) noexcept
		{
			using Model = CompleteModel<T&, Features...>;
			this->vtable = std::make_shared<Model>(obj);
		}
		template <typename T> requires ValidateType<T, Features...>
		explicit Any(const T& obj) noexcept
		{
			using Model = CompleteModel<const T&, Features...>;
			this->vtable = std::make_shared<Model>(obj);
		}
		template <typename T> requires ValidateType<T, Features...>
		explicit Any(std::reference_wrapper<T> obj) :
			Any{ obj.get() }
		{
		}

		template <typename T> requires ValidateType<T, Features...>
		auto GetObject() const -> const T&
		{
			using TNaked = std::remove_cvref_t<std::decay_t<T>>;

			if (IsCRef())
			{
				using Model = CompleteModel<std::add_lvalue_reference_t<std::add_const_t<TNaked>>, Features...>;
				const auto* model = dynamic_cast<const Model*>(GetModelPtr());
				return model->GetObject();
			}
			if (IsRef())
			{
				using Model = CompleteModel<std::add_lvalue_reference_t<TNaked>, Features...>;
				const auto* model = dynamic_cast<const Model*>(GetModelPtr());
				return model->GetObject();
			}

			using Model = CompleteModel<TNaked, Features...>;
			const auto* model = dynamic_cast<const Model*>(GetModelPtr());
			return model->GetObject();
		}

		template <typename T> requires ValidateType<T, Features...>
		auto GetObject() -> T&
		{
			if (!IsRef())
			{
				throw std::bad_any_cast{ };
			}
			if (IsCRef())
			{
				throw std::bad_any_cast{ };
			}

			using TNaked = std::remove_cvref_t<std::decay_t<T>>;
			using Model = CompleteModel<std::add_lvalue_reference_t<TNaked>, Features...>;
			const auto* model = dynamic_cast<Model*>(GetModelPtr());
			return model->GetObject();
		}

		template <typename T> requires ValidateType<T, Features...>
		auto Holds() const
		{
			return this->Type() == typeid(T);
		}

		template <FeatureType Feature>
		static consteval auto HasFeature() noexcept
		{
			return InParameterPack<Feature, Features...>;
		}

		constexpr auto IsRef() const noexcept
		{
			return this->vtable->IsRef();
		}
		constexpr auto IsCRef() const noexcept
		{
			return this->vtable->IsCRef();
		}

		constexpr auto GetTypeInformation() const
		{
			return AnyTypeInformation{
				.type = this->Type(),
				.isRef = this->IsRef(),
				.isCRef = this->IsCRef()
			};
		}

		private:
		constexpr auto GetModelPtr() const noexcept
		{
			return this->GetVTable();
		}
	};

	template <FeatureType... Features>
	struct FeatureComposer
	{
		template <typename T>
		struct Validator
		{
			static constexpr auto value = ValidateType<T, Features...>;
		};

		template <typename V>
		using VTable = VtableComposer<V, Features...>::Type;

		template <typename M>
		using Model = ModelComposer<M, Features...>::Type;

		template <typename I>
		using Interface = InterfaceComposer<I, Features...>::Type;
	};

	template <FeatureType... Features, typename T> requires ValidateType<T, Features...>
	auto MakeAny(T&& value) noexcept
	{
		return Any<Features...>(std::forward<T>(value));
	}
	template <FeatureType... Features, typename T> requires ValidateType<T, Features...>
	auto MakeAnyRef(T& value) noexcept
	{
		return Any<Features...>(std::ref(value));
	}
	template <FeatureType... Features, typename T> requires ValidateType<T, Features...>
	auto MakeAnyRef(const T& value) noexcept
	{
		return Any<Features...>(std::cref(value));
	}
	template <FeatureType... Features, typename T> requires ValidateType<T, Features...>
	auto MakeAnyRef(T&& value) noexcept
	{
		return Any<Features...>(std::move(static_cast<std::remove_cvref_t<T>&&>(value)));
	}
	template <FeatureType... Features, typename T> requires ValidateType<T, Features...>
	auto MakeAnyCRef(T& value) noexcept
	{
		return Any<Features...>(std::cref(value));
	}
	template <FeatureType... Features, typename T> requires ValidateType<T, Features...>
	auto MakeAnyCRef(const T& value) noexcept
	{
		return Any<Features...>(std::cref(value));
	}
	template <FeatureType... Features, typename T> requires ValidateType<T, Features...>
	auto MakeAnyCRef(T&& value) noexcept
	{
		return Any<Features...>(std::move(static_cast<std::remove_cvref_t<T>&&>(value)));
	}

	template <typename T, IsSpecialization<Any> AnyType>
		requires std::is_const_v<std::remove_reference_t<T>> || !std::is_lvalue_reference_v<T>
	auto AnyCast(const AnyType& any) -> const T&
	{
		return any.template GetObject<T>();
	}
	template <typename T, IsSpecialization<Any> AnyType>
		requires std::is_lvalue_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>
	auto AnyCast(AnyType& any) -> T&
	{
		return any.template GetObject<T>();
	}
}
