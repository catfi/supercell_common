#include <cstdio>
#include <vector>
#include <boost/shared_ptr.hpp>

using namespace boost;

template<class T>
bool operator == (const shared_ptr<T> &p, const T* t)
{
	return (p.get() == t);
}

class A
{
	public:

	A() { printf("ctor\n"); }
	virtual ~A() { printf("dtor\n"); }
};

class B
{
public:
	void setContext(shared_ptr<void> context)
	{
		mContext = context;
	}

	shared_ptr<void> getContext()
	{
		return mContext;
	}

private:
	shared_ptr<void> mContext;
};

int main()
{
	/*
	std::vector< shared_ptr<int> > cont;
	int* n = new int; *n = 10;
	shared_ptr<int> p(n);
	cont.push_back(p);

	shared_ptr<int> xx = (int*)0;

	std::vector< shared_ptr<int> >::iterator i = cont.begin();

	if(*i == n)
	{
		printf("error!!!\n");
	}
	else
	{
		printf("success!!\n");
	}
	*/

	B b;

	shared_ptr<void> x;

	{
		shared_ptr<void> p(new A());
		x = p;
	}

	b.setContext(shared_ptr<void>(new A()));

	return 0;
}
