#ifndef NODE_CLASS
#define NODE CLASS

#include <windows.h>
//declaration of Node Class

template <class T> class Node
{
public :
	Node <T> *next; // next part is a pointer to nodes of this type

	T *data; // data part is public
	// constructor
	Node (T *item, Node<T>* ptrNext=0){data = item; next = ptrNext;};

	//list modification methods
	Node<T> *GetLast()
		{
			Node<T> *last = this;
			while(last->next)
				last = last->next;
			return last;
		};

	void Delete(T *item)
		{
			{
				Node<T> *prev = 0;
				Node<T> *last = this;
				while(last)
				{
					if(last->data == item)
					{
						if(prev)
						{
							prev->next = last->next;
						}
						delete last;
						return;
					}

					prev = last;
					last = last->next;
				}
			}
		};

	Node<T> *GetNode(T *data)
	{
		Node<T> *last = this;
		while(last)
		{
			if(last->data == data)
				return last;
			last = last->next;
		}
		return 0;
	}

	int GetCount()
	{
		int count = 0;
		Node<T> *last = this;
		while(last)
		{
			if(last->data)
				count++;
			last = last->next;
		}
		return count;
	}
};

#endif