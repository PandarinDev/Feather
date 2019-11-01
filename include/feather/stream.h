#pragma once

#include <utility>
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

	template<typename T, typename R>
	struct collect_terminator {

		using value_type = T;
		using container_type = R;
		producer_type<value_type> generator;

		collect_terminator(const producer_type<value_type>& generator) :
			generator(generator) {}

		auto add_to_container(container_type& container, const value_type& entry) -> decltype(std::declval<R>().emplace_back()) {
			return container.emplace_back(entry);
		}

		R operator()() {
			container_type container;
			std::optional<value_type> next;
			while (next = generator()) {
				add_to_container(container, *next);
			}
			return container;
		}

	};

	template<typename T>
	struct count_terminator {

		using value_type = T;
		producer_type<value_type> generator;

		count_terminator(const producer_type<value_type>& generator) :
			generator(generator) {}

		std::size_t operator()() {
			std::size_t counter = 0;
			std::optional<value_type> next;
			while (next = generator()) {
				++counter;
			}
			return counter;
		}

	};

	template<typename T>
	struct any_match_terminator {

		using value_type = T;
		using predicate_type = std::function<bool(const value_type&)>;
		
		producer_type<value_type> generator;
		predicate_type predicate;

		any_match_terminator(
			const producer_type<value_type>& generator,
			const predicate_type& predicate) :
			generator(generator), predicate(predicate) {}

		bool operator()() {
			std::optional<value_type> next;
			while (next = generator()) {
				if (predicate(*next)) {
					return true;
				}
			}
			return false;
		}

	};

	template<typename T>
	struct all_match_terminator {

		using value_type = T;
		using predicate_type = std::function<bool(const value_type&)>;
		
		producer_type<value_type> generator;
		predicate_type predicate;

		all_match_terminator(
			const producer_type<value_type>& generator,
			const predicate_type& predicate) :
			generator(generator), predicate(predicate) {}

		bool operator()() {
			std::optional<value_type> next;
			while (next = generator()) {
				if (!predicate(*next)) {
					return false;
				}
			}
			return true;
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

		auto sum() const -> decltype(std::declval<T>() + std::declval<T>()) {
			return reduce(T{}, [](const auto& s, const auto& e) { return s + e; });
		}

		const std::optional<value_type> next() const {
			return producer();
		}

		template<typename Container>
		Container collect() const {
			return collect_terminator<T, Container>(producer)();
		}

		std::size_t count() const {
			return count_terminator<T>(producer)();
		}

		bool any_match(const typename any_match_terminator<T>::predicate_type& predicate) const {
			return any_match_terminator<T>(producer, predicate)();
		}

		bool all_match(const typename all_match_terminator<T>::predicate_type& predicate) const {
			return all_match_terminator<T>(producer, predicate)();
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
