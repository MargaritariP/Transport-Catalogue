#include "json_builder.h"

namespace json {

	Builder::Builder() {
		Node* root_ptr = &root_;
		nodes_stack_.emplace_back(root_ptr);
	}

	DictItemContext Builder::StartDict()
	{
		StartContainer(Dict());
		return *this;
	}

	ArrayItemContext Builder::StartArray()
	{
		StartContainer(Array());
		return *this;
	}

	Builder& Builder::EndDict()
	{
		if (!nodes_stack_.back()->IsDict()) {
			throw std::logic_error("No Map in the stack");
		}
		nodes_stack_.pop_back();
		key_.reset();
		return *this;
	}

	Builder& Builder::EndArray()
	{
		if (!nodes_stack_.back()->IsArray()) {
			throw std::logic_error("No Array in the stack");
		}
		nodes_stack_.pop_back();
		return *this;
	}

	KeyContext Builder::Key(std::string key)
	{
		if (nodes_stack_.back()->IsDict() && !key_) {
			key_ = std::move(key);
		}
		else {
			throw std::logic_error("Node is not a Map or Key has already been initialized");
		}
		return *this;
	}

	Builder& Builder::Value(Node::Value value)
	{
		if (nodes_stack_.back()->IsNull()) {
			nodes_stack_.back()->SetValue() = std::move(value);
			nodes_stack_.pop_back();
		}
		else if (nodes_stack_.back()->IsArray()) {
			Node temp;
			temp.SetValue() = std::move(value);
			std::get<Array>(nodes_stack_.back()->SetValue()).emplace_back(std::move(temp));
		}
		else if (nodes_stack_.back()->IsDict() && key_) {
			Node tmp;
			tmp.SetValue() = std::move(value);
			std::get<Dict>(nodes_stack_.back()->SetValue())[*key_] = std::move(tmp);
			key_.reset();
		}
		else if (nodes_stack_.empty()) {
			throw std::logic_error("Stack is empty");
		}
		else {
			throw std::logic_error("Node initialization error");
		}
		return *this;
	}

	Node Builder::Build()
	{
		if (!nodes_stack_.empty()) {
			throw std::logic_error("The stack is not completed");
		}
		return root_;
	}


	//~~~~~~~~~~~~~~~~~~Context~~~~~~~~~~~~~~~


	KeyContext DictItemContext::Key(std::string key)
	{
		return builder_.Key(std::move(key));
	}
	Builder& DictItemContext::EndDict()
	{
		return builder_.EndDict();
	}



	DictItemContext KeyContext::StartDict()
	{
		return builder_.StartDict();
	}
	ArrayItemContext KeyContext::StartArray()
	{
		return builder_.StartArray();
	}
	DictItemContext KeyContext::Value(Node::Value value)
	{
		return DictItemContext(builder_.Value(std::move(value)));
	}



	ArrayItemContext ArrayItemContext::Value(Node::Value value)
	{
		return ArrayItemContext(builder_.Value(std::move(value)));
	}
	DictItemContext ArrayItemContext::StartDict()
	{
		return builder_.StartDict();
	}
	ArrayItemContext ArrayItemContext::StartArray()
	{
		return builder_.StartArray();
	}
	Builder& ArrayItemContext::EndArray()
	{
		return builder_.EndArray();
	}

}//json