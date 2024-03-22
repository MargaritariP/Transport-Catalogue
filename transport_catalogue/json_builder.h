#pragma once

#include "json.h"
#include <optional>

namespace json {

	class DictItemContext;
	class KeyContext;
	class ArrayItemContext;

	class Builder {
	public:
		Builder();
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		KeyContext Key(std::string key);
		Builder& Value(Node::Value value);
		Node Build();

	private:
		Node root_ = nullptr;
		std::vector<Node*> nodes_stack_;
		std::optional<std::string> key_;


		template <typename Container>
		void StartContainer(Container container)
		{
			if (nodes_stack_.back()->IsNull()) {
				nodes_stack_.back()->SetValue() = std::move(container);
			}
			else if (nodes_stack_.back()->IsArray()) {
				auto& array = std::get<Array>(nodes_stack_.back()->SetValue());
				array.emplace_back(container);
				nodes_stack_.emplace_back(&array.back());
			}
			else if (nodes_stack_.back()->IsDict()) {
				if (!key_) {
					throw std::logic_error("No key in the Map");
				}
				auto& dict = std::get<Dict>(nodes_stack_.back()->SetValue());
				auto [pos, _] = dict.emplace(std::move(key_.value()), container);
				key_.reset();
				nodes_stack_.emplace_back(&pos->second);
			}
			else {
				throw std::logic_error("Can not start a container");
			}
		}
	};

	class DictItemContext {
	public:
		DictItemContext(Builder& builder)
			: builder_(builder)
		{
		}

		KeyContext Key(std::string key);
		Builder& EndDict();

	private:
		Builder& builder_;
	};

	class KeyContext {
	public:
		KeyContext(Builder& builder)
			:builder_(builder)
		{
		}

		DictItemContext StartDict();
		ArrayItemContext StartArray();
		DictItemContext Value(Node::Value value);
	private:
		Builder& builder_;
	};

	class ArrayItemContext {
	public:
		ArrayItemContext(Builder& builder)
			:builder_(builder)
		{
		}

		ArrayItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndArray();
	private:
		Builder& builder_;
	};

} //json
