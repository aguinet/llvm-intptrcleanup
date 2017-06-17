#include <algorithm>
#include <iterator>

#define RANDOM_IT

#ifdef RANDOM_IT
struct add_zip_iterator: public std::iterator<std::random_access_iterator_tag, int>
#else
struct add_zip_iterator: public std::iterator<std::forward_iterator_tag, int>
#endif
{
  int* a;
  int* b;

  using traits = std::iterator_traits<add_zip_iterator>;

  add_zip_iterator() { }
  add_zip_iterator(int* a_, int* b_):
    a(a_), b(b_)
  { }

  add_zip_iterator(add_zip_iterator const&) = default;
  add_zip_iterator(add_zip_iterator&&) = default;

  inline int operator*() const { return *a + *b; }
  inline add_zip_iterator& operator++() {
    ++a; ++b;
    return *this;
  }
  inline add_zip_iterator operator++(int) const {
    add_zip_iterator ret;
    ret.a = a+1;
    ret.b = b+1;
    return ret;
  }
  inline bool operator==(add_zip_iterator const& o) const
  {
    return (o.a == a) && (o.b == b);
  }
  inline bool operator!=(add_zip_iterator const& o) const
  {
    return !(*this == o);
  }
#ifdef RANDOM_IT
  inline add_zip_iterator& operator+=(traits::difference_type n)
  {
    a += n; b += n;
    return *this;
  }

  inline add_zip_iterator operator+(traits::difference_type n) const
  {
    return add_zip_iterator{a+n, b+n};
  }

  inline add_zip_iterator& operator-=(traits::difference_type n)
  {
    a -= n; b -= n;
    return *this;
  }

  inline add_zip_iterator operator-(traits::difference_type n) const
  {
    return add_zip_iterator{a-n, b-n};
  }

  inline int operator[](traits::difference_type n) const
  {
    return *(a+n) + *(b+n);
  }

  inline difference_type operator-(add_zip_iterator const& o) const
  {
    return o.a - a;
  }
#endif
};

void op_distance(int* res, add_zip_iterator const it, add_zip_iterator const it_end)
{
  const size_t n = std::distance(it, it_end);
  std::copy_n(it, n, res);
}

void op(int* res, add_zip_iterator const it, add_zip_iterator const it_end)
{
  std::copy(it, it_end, res);
}
