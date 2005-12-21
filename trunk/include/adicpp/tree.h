/*-
 * Copyright 2005 Guram Dukashvili
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
//---------------------------------------------------------------------------
#ifndef _tree_H
#define _tree_H
//-----------------------------------------------------------------------------
namespace ksys {
//-----------------------------------------------------------------------------
#define AVLTreeStackSize sizeof(int) * 8
//-----------------------------------------------------------------------------
/*template <class T>
class AVLTreeNode {
  private:
  protected:
  public:
    typedef long (*CmpFuncType)(T &,T &);
    enum NodeState { TL, WK, TR };
    enum LinkIDs { Left = 0, Right = 1, Prev = 0, Next = 1 };
    struct NodeStackEntry0 {
      AVLTreeNode<T> * Parent;
      AVLTreeNode<T> ** Node;
    };
    struct NodeStackEntry : public NodeStackEntry0 {
      NodeState State;
      LinkIDs ID;
    };
    AVLTreeNode<T> * Links[2];
    char SubTreeHeights[2];
    LinkIDs id;
    T Data;

    AVLTreeNode(const T & AData) : Data(AData) {
      Links[Left] = Links[Right] = NULL;
      SubTreeHeights[Left] = SubTreeHeights[Right] = 0;
    }
    char MaxSubTreeHeight() const { return SubTreeHeights[SubTreeHeights[Left] < SubTreeHeights[Right]]; }
    long Diff(LinkIDs left,LinkIDs right) const { return SubTreeHeights[left] - SubTreeHeights[right]; }
    void RotateBase(AVLTreeNode<T> * & RootPtr,LinkIDs left,LinkIDs right);
    AVLTreeNode<T> * InsertObject(AVLTreeNode<T> ** RootNode,CmpFuncType f,bool RetObj = false);
    AVLTreeNode<T> * Insert(AVLTreeNode<T> ** RootNode,CmpFuncType f){ return InsertObject(RootNode,f); }
    static void Delete(AVLTreeNode<T> ** RootNode,T * AData,CmpFuncType f);
    static void Clear(AVLTreeNode<T> ** RootNode);
    static AVLTreeNode<T> * Search(AVLTreeNode<T> * ANode,T * AData,CmpFuncType f);

    void RotateAfterInsert(AVLTreeNode<T> * & RootPtr,LinkIDs ID){
      LinkIDs left = LinkIDs(Right - ID);
      if( Diff(ID,left) > 1 ) RotateBase(RootPtr,left,ID);
    }

    enum TraverseDirectionType { Ascend = Left, Descend = Right };
    class Traverser {
      private:
      protected:
	long sp;
	AVLTreeNode<T> * RootNode;
	NodeStackEntry stack[AVLTreeStackSize * 2];
	TraverseDirectionType Direction;
      public:
	Traverser(AVLTreeNode<T> * ARootNode,TraverseDirectionType ADirection = Ascend);
	Traverser & Rewind(TraverseDirectionType ADirection = Ascend){
	  sp = 0;
	  Direction = ADirection;
	  stack[0].Parent = RootNode;
	  stack[0].State = TL;
	  return *this;
	}
	AVLTreeNode<T> * Next();
    };
};

template <class T>
void AVLTreeNode<T>::RotateBase(AVLTreeNode<T> * & RootPtr,LinkIDs left,LinkIDs right)
{
  RootPtr = Links[right];
  Links[right]->id = id;
  id = left;
  Links[right] = RootPtr->Links[left];
  RootPtr->Links[left] = this;
  if( Links[right] != NULL ) Links[right]->id = right;
  SubTreeHeights[right] = RootPtr->SubTreeHeights[left];
  RootPtr->SubTreeHeights[left] = char(MaxSubTreeHeight() + 1);
}

template <class T>
AVLTreeNode<T> * AVLTreeNode<T>::InsertObject(AVLTreeNode<T> ** RootNode,CmpFuncType f,bool RetObj)
{
  NodeStackEntry stack[AVLTreeStackSize], * p = stack;
  stack[0].Node = RootNode;
  while( *p->Node != NULL ){
    long c = f(Data,(*p->Node)->Data);
    if( c == 0 ){
      delete this;
      if( RetObj ) return *p->Node;
      return NULL;
    }
    (p + 1)->Parent = *p->Node;
    (p + 1)->Node = &(*p->Node)->Links[(p + 1)->ID = LinkIDs(c > 0)];
    p++;
  }
  *p->Node = this;
  id = p->ID;
  if( p - 2 >= stack &&
      p->Parent->SubTreeHeights[Right - p->ID] == 0 &&
      (p - 1)->Parent->SubTreeHeights[p->ID]   == 0 ){
    p->Parent->RotateBase(*(p - 1)->Node,LinkIDs(Right - p->ID),p->ID);
    (p - 1)->Parent->RotateBase(*(p - 2)->Node,p->ID,LinkIDs(Right - p->ID));
    p -= 2;
  }
  while( p > stack ){
    char s2 = char((*p->Node)->MaxSubTreeHeight() + 1);
    char & s1 = p->Parent->SubTreeHeights[p->ID];
    if( s1 != s2 ) s1 = s2;
    p->Parent->RotateAfterInsert(*(p - 1)->Node,p->ID);
    p--;
  }
  return this;
}

template <class T>
void AVLTreeNode<T>::Delete(AVLTreeNode<T> ** RootNode,T * AData,CmpFuncType f)
{
  long q;
  NodeStackEntry stack[AVLTreeStackSize], * p = stack;
  stack[0].Parent = NULL;
  stack[0].Node = RootNode;

  for(;;){
    if( *p->Node == NULL ) return;
    if( (q = f(*AData,(*p->Node)->Data)) == 0 ) break;
    (p + 1)->Parent = *p->Node;
    (p + 1)->Node = &(*p->Node)->Links[(p + 1)->ID = LinkIDs(q > 0)];
    p++;
  }
  LinkIDs l = Left, r = Right;
  AVLTreeNode<T> * Result = *p->Node, ** gl, ** gr, ** g;
  gl = &(*p->Node)->Links[Left];
  gr = &(*p->Node)->Links[Right];
  if( *(g = gl) == NULL ||
      (*gr != NULL && (*gr)->MaxSubTreeHeight() > (*gl)->MaxSubTreeHeight()) ){
    l = Right;
    r = Left;
    g = gr;
  }
  if( *g != NULL ){
    while( (*g)->Links[r] != NULL ) (*g)->RotateBase(*g,l,r);
    while( (*g)->Links[l] != NULL && (*g)->Links[l]->Diff(l,r) > 1 ){
      (*g)->Links[l]->RotateBase((*g)->Links[l],r,l);
      (*g)->SubTreeHeights[l]--;
    }
    (*g)->Links[r] = (*p->Node)->Links[r];
    if( (*g)->Links[r] != NULL ){
      (*g)->Links[r]->id = r;
      (*g)->SubTreeHeights[r] = char((*g)->Links[r]->MaxSubTreeHeight() + 1);
    }
    else {
      (*g)->SubTreeHeights[r] = 0;
    }
    (*g)->id = (*p->Node)->id;
    *p->Node = *g;
    if( (*g)->Links[r] != NULL ){
      (p + 1)->Parent = *p->Node;
      (p + 1)->Node = &(*p->Node)->Links[(p + 1)->ID = r];
      p++;
    }
  }
  else { // trap exception on Left and Right sibling is NULL
    if( p->Parent != NULL ) p->Parent->SubTreeHeights[(*p->Node)->id] = 0;
    *p->Node = NULL;
    p--;
  }
  while( p > stack ){
// helper code for operation system swapping, minimize dirty page writes
    char & h = p->Parent->SubTreeHeights[(*p->Node)->id];
    char m = char((*p->Node)->MaxSubTreeHeight() + 1);
// dirty page only if needed
    if( h != m ) h = m;
    q = p->Parent->Diff(Left,Right);
    if( q > 1 ){
      l = Right;
      r = Left;
    }
    else if( q < -1 ){
      l = Left;
      r = Right;
    }
    else {
      p--;
      continue;
    }
    p->Parent->RotateBase(*(p - 1)->Node,l,r);
    break;
  }
  delete Result;
  return;
}

template <class T>
AVLTreeNode<T>::Traverser::Traverser(
  AVLTreeNode<T> * ARootNode,
  TraverseDirectionType ADirection) : RootNode(ARootNode)
{
  Rewind(ADirection);
}

template <class T>
AVLTreeNode<T> * AVLTreeNode<T>::Traverser::Next()
{
  while( sp >= 0 ){
    if( stack[sp].Parent == NULL ) sp--;
    if( sp < 0 ) break;
    switch( stack[sp].State ){
      case TL :
	stack[sp + 1].Parent = stack[sp].Parent->Links[Direction];
	stack[sp + 1].State = TL;
	stack[sp].State = WK;
	sp++;
	break;
      case WK :
	stack[sp + 1].Parent = stack[sp].Parent->Links[Descend - Direction];
	stack[sp + 1].State = TL;
	stack[sp].State = TR;
	sp++;
	return stack[sp - 1].Parent;
      case TR :
	sp--;
	break;
    }
  }
  return NULL;
}

template <class T>
void AVLTreeNode<T>::Clear(AVLTreeNode<T> ** RootNode)
{
  AVLTreeNode<T> * p;
  Traverser trav(*RootNode);
  while( (p = trav.Next()) != NULL ) delete p;
  *RootNode = NULL;
}

template <class T>
AVLTreeNode<T> * AVLTreeNode<T>::Search(AVLTreeNode<T> * ANode,T * AData,CmpFuncType f)
{
  while( ANode != NULL ){
    long c = f(*AData,ANode->Data);
    if( c == 0 ) break;
    ANode = ANode->Links[LinkIDs(c > 0)];
  }
  return ANode;
}

template <class T>
class AVLTree {
  private:
    static void Destroyer(AVLTreeNode<T> * ANode);
  protected:
  public:
    typedef long (*CmpFuncType)(T &,T &);
    CmpFuncType f;
    AVLTreeNode<T> * RootNode;

    AVLTree();
    AVLTree(CmpFuncType Af);
    ~AVLTree();

    bool IsEmpty() const { return RootNode == NULL; }

    AVLTreeNode<T> * Insert(const T AData);
    AVLTreeNode<T> * InsertObject(const T AData);
    AVLTreeNode<T> * Search(const T AData) const;
    AVLTree<T> & Delete(const T AData);
    AVLTree<T> & Clear(){ AVLTreeNode<T>::Clear(&RootNode); return *this; }

    enum TraverseDirectionType { Ascend = AVLTreeNode<T>::Ascend, Descend = AVLTreeNode<T>::Ascend };
    class Traverser : public AVLTreeNode<T>::Traverser {
      private:
      protected:
      public:
        Traverser(AVLTree<T> * ATree,TraverseDirectionType ADirection = Ascend);
        Traverser & Rewind(TraverseDirectionType ADirection = Ascend){ AVLTreeNode<T>::Traverser::Rewind(AVLTreeNode<T>::TraverseDirectionType(ADirection)); return *this;}
        AVLTreeNode<T> * Next(){ return AVLTreeNode<T>::Traverser::Next(); }
        T * operator -> () const { return &stack[sp - 1].Parent->Data; }
        T & Data() const { return stack[sp - 1].Parent.Data->Data; }
    };

    AVLTree<T> & Assign(const AVLTree<T> & s);
    AVLTree<T> & Insert(const AVLTree<T> & s);
};

template <class T>
AVLTree<T> & AVLTree<T>::Assign(const AVLTree<T> & s)
{
  Clear();
  f = s.f;
  AVLTreeNode<T> * p;
  Traverser trav(const_cast<AVLTree<T> *>(&s));
  while( (p = trav.Next()) != NULL ) Insert(p->Data);
  return *this;
}

template <class T>
AVLTree<T> & AVLTree<T>::Insert(const AVLTree<T> & s)
{
  AVLTreeNode<T> * p;
  Traverser trav(const_cast<AVLTree<T> *>(&s));
  while( (p = trav.Next()) != NULL ) Insert(p->Data);
  return *this;
}

template <class T>
AVLTree<T> & AVLTree<T>::Delete(const T AData)
{
  AVLTreeNode<T>::Delete(&RootNode,const_cast<T *>(&AData),f);
  return *this;
}

template <class T>
AVLTreeNode<T> * AVLTree<T>::Search(const T AData) const
{
  return AVLTreeNode<T>::Search(RootNode,const_cast<T *>(&AData),f);
}

template <class T>
AVLTree<T>::AVLTree() : Component(NULL), f(NULL), RootNode(NULL)
{
}

template <class T>
AVLTree<T>::AVLTree(CmpFuncType Af) : f(Af), RootNode(NULL)
{
}

template <class T>
AVLTree<T>::~AVLTree()
{
  Clear();
}

template <class T>
AVLTree<T>::Traverser::Traverser(
  AVLTree<T> * ATree,TraverseDirectionType ADirection) :
  AVLTreeNode<T>::Traverser(ATree->RootNode,AVLTreeNode<T>::TraverseDirectionType(ADirection))
{
}

template <class T> inline
void AVLTree<T>::Destroyer(AVLTreeNode<T> * ANode)
{
  delete ANode;
}

template <class T>
AVLTreeNode<T> * AVLTree<T>::Insert(const T AData)
{
  AVLTreeNodeLink<LT> ANode = new AVLTreeNode<T>(AData);
  return ANode->Insert(&RootNode,f);
}

template <class T>
AVLTreeNode<T> * AVLTree<T>::InsertObject(const T AData)
{
  AVLTreeNode<T> * ANode = new AVLTreeNode<T>(AData);
  AVLTreeNode<T> * ANode2 = ANode->InsertObject(&RootNode,f,true);
  if( ANode != ANode2 ) delete AData;
  return ANode2;
}*/
//-----------------------------------------------------------------------------
#undef AVLTreeStackSize
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
#endif
