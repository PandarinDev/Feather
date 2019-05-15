#pragma once

#include <optional>
#include <functional>

namespace feather {

	template<typename T>
	using producer_type = std::function<std::optional<T>()>;

	template<typename Container>
	struct default_producer {

		using value_type = typename Container::value_type;
		using iterator_type = typename Container::const_iterator;

		iterator_type it;
		iterator_type end;

		default_producer(const Container& container) :
			it(std::cbegin(container)),
	   		end(std::cend(container)) {}

		std::optional<value_type> operator()() {
			return it != end
				? std::make_optional(*(it++))
				: std::nullopt;
		}

	};

	template<typename T>
	struct filter_producer {

		using value_type = T;
		using predicate_type = std::function<bool(const value_type&)>;

		producer_type<value_type> generator;
		predicate_type predicate;

		filter_producer(
				const producer_type<value_type>& generator,
				const predicate_type& predicate) :
			generator(generator),
			predicate(predicate) {}

		std::optional<value_type> operator()() {
			std::optional<value_type> next;
			while ((next = generator()) && !predicate(*next));
			return next;
		}

	};

	template<typename T, typename R>
	struct map_producer {

		using source_type = T;
		using result_type = R;
		using mapping_type = std::function<R(const T&)>;

		producer_type<source_type> generator;
		mapping_type mapper;

		map_producer(
				const producer_type<source_type>& generator,
				const mapping_type& mapper) :
			generator(generator),
			mapper(mapper) {}

		std::optional<result_type> operator()() {
			auto next = generator();
			if (!next) {
				return std::nullopt;
			}
			return std::make_optional(mapper(*next));
		}

	};

	template<typename T, typename R>
	struct reduce_terminator {

		using source_type = T;
		using result_type = R;
		using reduction_type = std::function<R(const R&, const T&)>;

		producer_type<source_type> generator;
		reduction_type reducer;

		reduce_terminator(
				const producer_type<source_type>& generator,
				const reduction_type& reducer) :
			generator(generator),
			reducer(reducer) {}

		R operator()(R seed) {
			std::optional<T> next;
			while (next = generator()) {
				seed = reducer(seed, *next);
			}
			return seed;
		}

	};

	template<typename T>
	struct stream_type {

		using value_type = T;

		stream_type(const producer_type<T>& producer) : producer(producer) {}

		stream_type<T> filter(const typename filter_producer<T>::predicate_type& predicate) const {
			return stream_type(filter_producer<T>(producer, predicate));
		}

		template<typename Mapper>
		auto map(const Mapper& mapper) const {
			return stream_type(map_producer<T, std::invoke_result_t<Mapper, const T&>>(producer, mapper));
		}

		template<typename Seed, typename Reducer>
		Seed reduce(const Seed& seed, const Reducer& reducer) const {
			return reduce_terminator<T, Seed>(producer, reducer)(seed);
		}

		T sum() const {
			return reduce(T{}, [](const auto& s, const auto& e) { return s + e; });
		}

		const std::optional<value_type> next() const {
			return producer();
		}

	private:

		const producer_type<T> producer;

	};

	template<typename Collection>
	static auto Stream(const Collection& collection) {
		return stream_type<typename Collection::value_type>(default_producer(collection));
	}

	template<typename T>
	static auto Stream(std::initializer_list<T> values) {
		return stream_type<T>(default_producer(values));
	}

}
