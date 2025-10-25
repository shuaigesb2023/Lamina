//类型说明在docs/zh_CN/wiki.md下

#ifndef LAMINALIST
#define LAMINALIST

#include <stdexcept>
#include <string>

namespace __LAMINA::ListType {

	template <class VT>//value type
	class base_list {
	public:

		class base_list_node {
		public:

			VT value;
			base_list_node* next;

			base_list_node() :value(), next(nullptr) {}
			base_list_node(base_list_node* n) : value(VT()), next(n) {}
			base_list_node(const VT& val, base_list_node* n) :value(val), next(n) {}

		};

		inline base_list_node* safe_new(base_list_node* i) {
			if (i == nullptr)throw std::runtime_error("Could not make node");
			return i;
		}

		base_list_node* root;
		base_list_node* end;

		base_list() { root = safe_new(new base_list_node()); end = root; }
		base_list(const VT& val) :base_list() { push_back(val); }
		base_list(const base_list& other) :base_list() {
			*this = other;
		}
		base_list(const std::initializer_list<VT>& initl) :base_list() {
			*this = initl;
		}

		inline void push_back(const VT& val) {
			end->next = safe_new(new base_list_node(val,nullptr));
			end = end->next;
		}

		inline void push_back(const base_list<VT>& other) {
			if (this == &other) {
				base_list<VT> t = other;
				end->next = t.root->next;
				end = t.end;
				delete t.root;
				return;
			}
			base_list_node* t = other.root->next;
			while (t) {
				push_back(t->value);
				t = t->next;
			}
		}

		inline void push_front(const VT& val) {
			base_list_node* t = safe_new(new base_list_node(val,root->next));
			root->next = t;
		}

		inline void push_front(const base_list<VT>& other) {
			base_list<VT> t = *this;
			*this = other;
			end->next = t.root->next;
			end = t.end;
			delete t.root;
		}

		inline void insert(size_t index,const VT& val) {
			push_ones_back(get_last_node(index), val);
		}

		inline void insert(size_t index, const base_list<VT>& other) {
			if (other.root == other.end)return;
			push_ones_back(get_last_node(index), other);
		}

		//插入区间，左闭右开
		inline void insert(size_t index, const base_list<VT>& other,size_t begini,size_t endi) {
			base_list_node* t = get_last_node(index);
			base_list<VT> listt = other.get_range(begini,endi);
			listt.end->next = t->next;
			t->next = listt.root->next;
			delete listt.root;
		}

		inline void pop_back() {
			if (root == end)throw std::runtime_error("pop from empty list");
			base_list_node* t = root;
			while (t->next == end) {
				t = t->next;
			}
			delete t->next;
			t->next = nullptr;
		}

		inline void pop_front() {
			if (root == end)throw std::runtime_error("pop from empty list");
			base_list_node* del_node = root->next;
			root->next = root->next->next;
			delete del_node;
		}

		inline void pop(size_t index) {
			if (root == end)throw std::runtime_error("pop from empty list");
			base_list_node* t = root;
			while (index) {
				t = t->next;
				if (t == nullptr)throw std::runtime_error("Index out for range");
				index--;
			}
			base_list_node* del_node = t->next;
			t->next = t->next->next;
			delete del_node;
		}

		//删除区间，左闭右开
		inline void pop(size_t begini,size_t endi) {
			if (endi <= begini)throw std::runtime_error("End must greater than begin");
			base_list_node* t = get_last_node(begini);
			while (begini != endi) {
				pop_ones_next(t);
				endi--;
			}
		}

		inline void eraser(size_t index) { pop(index); }

		inline void eraser(size_t begini, size_t endi) { pop(begini, endi); }

		//获取区间，左闭右开
		inline base_list<VT> get_range(size_t begini, size_t endi) const{
			if (endi <= begini)throw std::runtime_error("End must greater than begin");
			base_list<VT> re;
			base_list_node* t = root;
			while (begini) {
				t = t->next;
				if (t == nullptr)throw std::runtime_error("Index out for range");
				begini--;
				endi--;
			}
			while (endi) {
				re += t->next->value;
				t = t->next;
				if (t == nullptr)throw std::runtime_error("Index out for range");
				endi--;
			}
			return re;
		}

		inline void clear() {
			while (root != end)pop_front();
		}

		inline void operator+=(const VT& val) {
			push_back(val);
		}

		inline void operator+=(const base_list<VT>& other) {
			push_back(other);
		}

		inline base_list<VT> operator+(const VT& val) const{
			base_list<VT> t = *this;
			t.push_back(val);
			return t;
		}

		inline base_list<VT> operator+(const base_list<VT>& other) const{
			base_list<VT> t = *this;
			t += other;
			return t;
		}

		inline VT& operator[](size_t index) {
			base_list_node* t = root->next;
			while (index) {
				t = t->next;
				if (t == nullptr)throw std::runtime_error("Index out for range");
				index--;
			}
			return t->value;
		}

		inline void operator=(const base_list<VT>& other) {
			if (this == &other)return;
			clear();
			base_list_node* t = other.root->next;
			while (t) {
				push_back(t->value);
				t = t->next;
			}
		}

		inline void operator=(const std::initializer_list<VT>& initl) {
			clear();
			for (const VT& i : initl) {
				push_back(i);
			}
		}


	private:

		inline void push_ones_back(base_list_node* one, const VT& val) {
			base_list_node* t = safe_new(new base_list_node(val, one->next));
			one->next = t;
		}

		inline void pop_ones_next(base_list_node* one) {
			base_list_node* t = one->next;
			one->next = one->next->next;
			delete t;
		}

		inline void push_ones_back(base_list_node* one, const base_list<VT>& other) {
			base_list<VT> t = other;
			t.end->next = one->next;
			one->next = t.root->next;
		}

		inline base_list_node* get_last_node(size_t index) {
			base_list_node* t = root;
			while (index) {
				t = t->next;
				if (t == nullptr)throw std::runtime_error("Index out for range");
				index--;
			}
			return t;
		}

	};

}


#endif // !LAMINALIST
