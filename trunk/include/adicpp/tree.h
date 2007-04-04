/*-
 * Copyright 2007 Guram Dukashvili
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
#ifndef _EmbeddedTree_H
#define _EmbeddedTree_H
//-----------------------------------------------------------------------------
namespace ksys {
//-----------------------------------------------------------------------------
#define AVLEmbeddedTreeStackSize sizeof(uintptr_t) * 8
//-----------------------------------------------------------------------------
/*template <class T>
class AVLEmbeddedTreeNode {
  private:
  protected:
  public:
    typedef long (*CmpFuncType)(T &,T &);
    enum NodeState { TL, WK, TR };
    enum LinkIDs { Left = 0, Right = 1, Prev = 0, Next = 1 };
    struct NodeStackEntry0 {
      AVLEmbeddedTreeNode<T> * Parent;
      AVLEmbeddedTreeNode<T> ** Node;
    };
    struct NodeStackEntry : public NodeStackEntry0 {
      NodeState State;
      LinkIDs ID;
    };
    AVLEmbeddedTreeNode<T> * Links[2];
    char SubEmbeddedTreeHeights[2];
    LinkIDs id;
    T Data;

    AVLEmbeddedTreeNode(const T & AData) : Data(AData) {
      Links[Left] = Links[Right] = NULL;
      SubEmbeddedTreeHeights[Left] = SubEmbeddedTreeHeights[Right] = 0;
    }
    char MaxSubEmbeddedTreeHeight() const { return SubEmbeddedTreeHeights[SubEmbeddedTreeHeights[Left] < SubEmbeddedTreeHeights[Right]]; }
    long Diff(LinkIDs left,LinkIDs right) const { return SubEmbeddedTreeHeights[left] - SubEmbeddedTreeHeights[right]; }
    void RotateBase(AVLEmbeddedTreeNode<T> * & RootPtr,LinkIDs left,LinkIDs right);
    AVLEmbeddedTreeNode<T> * InsertObject(AVLEmbeddedTreeNode<T> ** RootNode,CmpFuncType f,bool RetObj = false);
    AVLEmbeddedTreeNode<T> * Insert(AVLEmbeddedTreeNode<T> ** RootNode,CmpFuncType f){ return InsertObject(RootNode,f); }
    static void Delete(AVLEmbeddedTreeNode<T> ** RootNode,T * AData,CmpFuncType f);
    static void Clear(AVLEmbeddedTreeNode<T> ** RootNode);
    static AVLEmbeddedTreeNode<T> * Search(AVLEmbeddedTreeNode<T> * ANode,T * AData,CmpFuncType f);

    void RotateAfterInsert(AVLEmbeddedTreeNode<T> * & RootPtr,LinkIDs ID){
      LinkIDs left = LinkIDs(Right - ID);
      if( Diff(ID,left) > 1 ) RotateBase(RootPtr,left,ID);
    }

    enum TraverseDirectionType { Ascend = Left, Descend = Right };
    class Traverser {
      private:
      protected:
  long sp;
  AVLEmbeddedTreeNode<T> * RootNode;
  NodeStackEntry stack[AVLEmbeddedTreeStackSize * 2];
  TraverseDirectionType Direction;
      public:
  Traverser(AVLEmbeddedTreeNode<T> * ARootNode,TraverseDirectionType ADirection = Ascend);
  Traverser & Rewind(TraverseDirectionType ADirection = Ascend){
    sp = 0;
    Direction = ADirection;
    stack[0].Parent = RootNode;
    stack[0].State = TL;
    return *this;
  }
  AVLEmbeddedTreeNode<T> * Next();
    };
};

template <class T>
void AVLEmbeddedTreeNode<T>::RotateBase(AVLEmbeddedTreeNode<T> * & RootPtr,LinkIDs left,LinkIDs right)
{
  RootPtr = Links[right];
  Links[right]->id = id;
  id = left;
  Links[right] = RootPtr->Links[left];
  RootPtr->Links[left] = this;
  if( Links[right] != NULL ) Links[right]->id = right;
  SubEmbeddedTreeHeights[right] = RootPtr->SubEmbeddedTreeHeights[left];
  RootPtr->SubEmbeddedTreeHeights[left] = char(MaxSubEmbeddedTreeHeight() + 1);
}

template <class T>
AVLEmbeddedTreeNode<T> * AVLEmbeddedTreeNode<T>::InsertObject(AVLEmbeddedTreeNode<T> ** RootNode,CmpFuncType f,bool RetObj)
{
  NodeStackEntry stack[AVLEmbeddedTreeStackSize], * p = stack;
  stack[0].Node = RootNode;
  while( *p->Node != NULL ){
    long c = f(Data,(*p->Node)->Data);
    if( c == 0 ){
      deleteObject(this);
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
      p->Parent->SubEmbeddedTreeHeights[Right - p->ID] == 0 &&
      (p - 1)->Parent->SubEmbeddedTreeHeights[p->ID]   == 0 ){
    p->Parent->RotateBase(*(p - 1)->Node,LinkIDs(Right - p->ID),p->ID);
    (p - 1)->Parent->RotateBase(*(p - 2)->Node,p->ID,LinkIDs(Right - p->ID));
    p -= 2;
  }
  while( p > stack ){
    char s2 = char((*p->Node)->MaxSubEmbeddedTreeHeight() + 1);
    char & s1 = p->Parent->SubEmbeddedTreeHeights[p->ID];
    if( s1 != s2 ) s1 = s2;
    p->Parent->RotateAfterInsert(*(p - 1)->Node,p->ID);
    p--;
  }
  return this;
}

template <class T>
void AVLEmbeddedTreeNode<T>::Delete(AVLEmbeddedTreeNode<T> ** RootNode,T * AData,CmpFuncType f)
{
  long q;
  NodeStackEntry stack[AVLEmbeddedTreeStackSize], * p = stack;
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
  AVLEmbeddedTreeNode<T> * Result = *p->Node, ** gl, ** gr, ** g;
  gl = &(*p->Node)->Links[Left];
  gr = &(*p->Node)->Links[Right];
  if( *(g = gl) == NULL ||
      (*gr != NULL && (*gr)->MaxSubEmbeddedTreeHeight() > (*gl)->MaxSubEmbeddedTreeHeight()) ){
    l = Right;
    r = Left;
    g = gr;
  }
  if( *g != NULL ){
    while( (*g)->Links[r] != NULL ) (*g)->RotateBase(*g,l,r);
    while( (*g)->Links[l] != NULL && (*g)->Links[l]->Diff(l,r) > 1 ){
      (*g)->Links[l]->RotateBase((*g)->Links[l],r,l);
      (*g)->SubEmbeddedTreeHeights[l]--;
    }
    (*g)->Links[r] = (*p->Node)->Links[r];
    if( (*g)->Links[r] != NULL ){
      (*g)->Links[r]->id = r;
      (*g)->SubEmbeddedTreeHeights[r] = char((*g)->Links[r]->MaxSubEmbeddedTreeHeight() + 1);
    }
    else {
      (*g)->SubEmbeddedTreeHeights[r] = 0;
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
    if( p->Parent != NULL ) p->Parent->SubEmbeddedTreeHeights[(*p->Node)->id] = 0;
    *p->Node = NULL;
    p--;
  }
  while( p > stack ){
// helper code for operation system swapping, minimize dirty page writes
    char & h = p->Parent->SubEmbeddedTreeHeights[(*p->Node)->id];
    char m = char((*p->Node)->MaxSubEmbeddedTreeHeight() + 1);
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
  deleteObject(Result);
  return;
}

template <class T>
AVLEmbeddedTreeNode<T>::Traverser::Traverser(
  AVLEmbeddedTreeNode<T> * ARootNode,
  TraverseDirectionType ADirection) : RootNode(ARootNode)
{
  Rewind(ADirection);
}

template <class T>
AVLEmbeddedTreeNode<T> * AVLEmbeddedTreeNode<T>::Traverser::Next()
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
void AVLEmbeddedTreeNode<T>::Clear(AVLEmbeddedTreeNode<T> ** RootNode)
{
  AVLEmbeddedTreeNode<T> * p;
  Traverser trav(*RootNode);
  while( (p = trav.Next()) != NULL ) deleteObject(p);
  *RootNode = NULL;
}

template <class T>
AVLEmbeddedTreeNode<T> * AVLEmbeddedTreeNode<T>::Search(AVLEmbeddedTreeNode<T> * ANode,T * AData,CmpFuncType f)
{
  while( ANode != NULL ){
    long c = f(*AData,ANode->Data);
    if( c == 0 ) break;
    ANode = ANode->Links[LinkIDs(c > 0)];
  }
  return ANode;
}

template <class T>
class AVLEmbeddedTree {
  private:
    static void Destroyer(AVLEmbeddedTreeNode<T> * ANode);
  protected:
  public:
    typedef long (*CmpFuncType)(T &,T &);
    CmpFuncType f;
    AVLEmbeddedTreeNode<T> * RootNode;

    AVLEmbeddedTree();
    AVLEmbeddedTree(CmpFuncType Af);
    ~AVLEmbeddedTree();

    bool IsEmpty() const { return RootNode == NULL; }

    AVLEmbeddedTreeNode<T> * Insert(const T AData);
    AVLEmbeddedTreeNode<T> * InsertObject(const T AData);
    AVLEmbeddedTreeNode<T> * Search(const T AData) const;
    AVLEmbeddedTree<T> & Delete(const T AData);
    AVLEmbeddedTree<T> & Clear(){ AVLEmbeddedTreeNode<T>::Clear(&RootNode); return *this; }

    enum TraverseDirectionType { Ascend = AVLEmbeddedTreeNode<T>::Ascend, Descend = AVLEmbeddedTreeNode<T>::Ascend };
    class Traverser : public AVLEmbeddedTreeNode<T>::Traverser {
      private:
      protected:
      public:
        Traverser(AVLEmbeddedTree<T> * AEmbeddedTree,TraverseDirectionType ADirection = Ascend);
        Traverser & Rewind(TraverseDirectionType ADirection = Ascend){ AVLEmbeddedTreeNode<T>::Traverser::Rewind(AVLEmbeddedTreeNode<T>::TraverseDirectionType(ADirection)); return *this;}
        AVLEmbeddedTreeNode<T> * Next(){ return AVLEmbeddedTreeNode<T>::Traverser::Next(); }
        T * operator -> () const { return &stack[sp - 1].Parent->Data; }
        T & Data() const { return stack[sp - 1].Parent.Data->Data; }
    };

    AVLEmbeddedTree<T> & Assign(const AVLEmbeddedTree<T> & s);
    AVLEmbeddedTree<T> & Insert(const AVLEmbeddedTree<T> & s);
};

template <class T>
AVLEmbeddedTree<T> & AVLEmbeddedTree<T>::Assign(const AVLEmbeddedTree<T> & s)
{
  Clear();
  f = s.f;
  AVLEmbeddedTreeNode<T> * p;
  Traverser trav(const_cast<AVLEmbeddedTree<T> *>(&s));
  while( (p = trav.Next()) != NULL ) Insert(p->Data);
  return *this;
}

template <class T>
AVLEmbeddedTree<T> & AVLEmbeddedTree<T>::Insert(const AVLEmbeddedTree<T> & s)
{
  AVLEmbeddedTreeNode<T> * p;
  Traverser trav(const_cast<AVLEmbeddedTree<T> *>(&s));
  while( (p = trav.Next()) != NULL ) Insert(p->Data);
  return *this;
}

template <class T>
AVLEmbeddedTree<T> & AVLEmbeddedTree<T>::Delete(const T AData)
{
  AVLEmbeddedTreeNode<T>::Delete(&RootNode,const_cast<T *>(&AData),f);
  return *this;
}

template <class T>
AVLEmbeddedTreeNode<T> * AVLEmbeddedTree<T>::Search(const T AData) const
{
  return AVLEmbeddedTreeNode<T>::Search(RootNode,const_cast<T *>(&AData),f);
}

template <class T>
AVLEmbeddedTree<T>::AVLEmbeddedTree() : Component(NULL), f(NULL), RootNode(NULL)
{
}

template <class T>
AVLEmbeddedTree<T>::AVLEmbeddedTree(CmpFuncType Af) : f(Af), RootNode(NULL)
{
}

template <class T>
AVLEmbeddedTree<T>::~AVLEmbeddedTree()
{
  Clear();
}

template <class T>
AVLEmbeddedTree<T>::Traverser::Traverser(
  AVLEmbeddedTree<T> * AEmbeddedTree,TraverseDirectionType ADirection) :
  AVLEmbeddedTreeNode<T>::Traverser(AEmbeddedTree->RootNode,AVLEmbeddedTreeNode<T>::TraverseDirectionType(ADirection))
{
}

template <class T> inline
void AVLEmbeddedTree<T>::Destroyer(AVLEmbeddedTreeNode<T> * ANode)
{
  deleteObject(ANode);
}

template <class T>
AVLEmbeddedTreeNode<T> * AVLEmbeddedTree<T>::Insert(const T AData)
{
  AVLEmbeddedTreeNodeLink<LT> ANode = newObject<AVLEmbeddedTreeNode<T> >(AData);
  return ANode->Insert(&RootNode,f);
}

template <class T>
AVLEmbeddedTreeNode<T> * AVLEmbeddedTree<T>::InsertObject(const T AData)
{
  AVLEmbeddedTreeNode<T> * ANode = newObject<AVLEmbeddedTreeNode<T> >(AData);
  AVLEmbeddedTreeNode<T> * ANode2 = ANode->InsertObject(&RootNode,f,true);
  if( ANode != ANode2 ) deleteObject(AData);
  return ANode2;
}*/
//-----------------------------------------------------------------------------
#undef AVLEmbeddedTreeStackSize
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <typename T>
class EmbeddedTreeNode {
  public:
    ~EmbeddedTreeNode();
    EmbeddedTreeNode();

    union {
      struct {
        EmbeddedTreeNode<T> * left_;
        EmbeddedTreeNode<T> * right_;
      };
      EmbeddedTreeNode<T> * leafs_[2];
    };
    intptr_t leaf_;

    bool isLeafs() const;
    bool isNullLeafs() const;
    T & object(const EmbeddedTreeNode<T> & node) const;
  protected:
  private:
};
//-----------------------------------------------------------------------------
template <typename T> inline
EmbeddedTreeNode<T>::~EmbeddedTreeNode()
{
}
//-----------------------------------------------------------------------------
template <typename T> inline
EmbeddedTreeNode<T>::EmbeddedTreeNode()
{
}
//-----------------------------------------------------------------------------
template <typename T> inline
bool EmbeddedTreeNode<T>::isLeafs() const
{
  return left_ != NULL || right_ != NULL;
}
//-----------------------------------------------------------------------------
template <typename T> inline
bool EmbeddedTreeNode<T>::isNullLeafs() const
{
  return left_ == NULL && right_ == NULL;
}
//-----------------------------------------------------------------------------
template <typename T> inline
T & EmbeddedTreeNode<T>::object(const EmbeddedTreeNode<T> & node) const
{
  return *(T *) const_cast<uint8_t *>(
    (const uint8_t *) this - (uintptr_t) const_cast<EmbeddedTreeNode<T> *>(&node)
  );
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <typename T>
class EmbeddedTreeNode3 : public EmbeddedTreeNode<T> {
  public:
    ~EmbeddedTreeNode3();
    EmbeddedTreeNode3();

    EmbeddedTreeNode3<T> * parent_;
  protected:
  private:
};
//-----------------------------------------------------------------------------
template <typename T> inline
EmbeddedTreeNode3<T>::~EmbeddedTreeNode3()
{
}
//-----------------------------------------------------------------------------
template <typename T> inline
EmbeddedTreeNode3<T>::EmbeddedTreeNode3()
{
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <
  typename T,
  EmbeddedTreeNode<T> & (*N)(const T &),
  T & (*O) (const EmbeddedTreeNode<T> &,T *),
  intptr_t (*C)(const T &,const T &)
>
class EmbeddedTree {
  public:
    ~EmbeddedTree();
    EmbeddedTree();

    EmbeddedTree<T,N,O,C> & clear();
    EmbeddedTree<T,N,O,C> & insert(const T & object,bool throwIfExist = true,bool deleteIfExist = true,T ** pObject = NULL);
    EmbeddedTree<T,N,O,C> & remove(const T & object,bool throwIfNotExist = true,bool deleteIfNotExist = false,T ** pObject = NULL);
    EmbeddedTree<T,N,O,C> & drop(const T & object,bool throwIfNotExist = true,bool deleteIfNotExist = false);
  protected:
  public:
    EmbeddedTree<T,N,O,C> & saveEmbeddedTreeGraph(AsyncFile & file) const;
    class Stack {
      public:
        ~Stack() {}
        Stack() : sp_(0) {}

        EmbeddedTreeNode<T> ** node_[sizeof(uintptr_t) * 8];
        intptr_t leaf_[sizeof(uintptr_t) * 8];
        intptr_t sp_;
    };

    EmbeddedTreeNode<T> * root_;
    uintptr_t count_;

    static uintptr_t c2i(intptr_t c){ return (c >> (sizeof(c) * 8 - 1)) + 1; }
    T * internalFind(EmbeddedTreeNode<T> ** pNode,const T & object,bool throwIfExist,bool throwIfNotExist,bool deleteIfExist,bool deleteIfNotExist,Stack * pStack = NULL) const;
    void rotate(EmbeddedTreeNode<T> ** pNode,intptr_t c);
  private:
};
//-----------------------------------------------------------------------------
template <
  typename T,
  EmbeddedTreeNode<T> & (*N)(const T &),
  T & (*O) (const EmbeddedTreeNode<T> &,T *),
  intptr_t (*C)(const T &,const T &)
> inline
EmbeddedTree<T,N,O,C>::~EmbeddedTree()
{
}
//-----------------------------------------------------------------------------
template <
  typename T,
  EmbeddedTreeNode<T> & (*N)(const T &),
  T & (*O) (const EmbeddedTreeNode<T> &,T *),
  intptr_t (*C)(const T &,const T &)
> inline
EmbeddedTree<T,N,O,C>::EmbeddedTree()
{
  clear();
}
//-----------------------------------------------------------------------------
template <
  typename T,
  EmbeddedTreeNode<T> & (*N)(const T &),
  T & (*O) (const EmbeddedTreeNode<T> &,T *),
  intptr_t (*C)(const T &,const T &)
> inline
EmbeddedTree<T,N,O,C> & EmbeddedTree<T,N,O,C>::clear()
{
  root_ = NULL;
  count_ = 0;
  return *this;
}
//-----------------------------------------------------------------------------
template <
  typename T,
  EmbeddedTreeNode<T> & (*N)(const T &),
  T & (*O) (const EmbeddedTreeNode<T> &,T *),
  intptr_t (*C)(const T &,const T &)
> inline
T * EmbeddedTree<T,N,O,C>::internalFind(
  EmbeddedTreeNode<T> ** pNode,
  const T & object,
  bool throwIfExist,
  bool throwIfNotExist,
  bool deleteIfExist,
  bool deleteIfNotExist,
  Stack * pStack) const
{
  T * pObject = NULL;
  for(;;){
    if( *pNode == NULL ) break;
    intptr_t c = C(object,O(**pNode,NULL));
    if( pStack != NULL ){
      pStack->node_[pStack->sp_] = pNode;
      pStack->leaf_[pStack->sp_++] = c;
    }
    if( c == 0 ){
      pObject = &O(**pNode,NULL);
      break;
    }
    pNode = &(*pNode)->leafs_[c2i(c)];
  }
  int32_t err = 0;
  if( pObject != NULL ){
    if( deleteIfExist ) deleteObject(&object);
    if( throwIfExist ){
#if defined(__WIN32__) || defined(__WIN64__)
      err = ERROR_ALREADY_EXISTS;
#else
      err = EEXIST;
#endif
    }
  }
  else if( pObject == NULL ){
    if( deleteIfNotExist ) deleteObject(&object);
    if( throwIfNotExist ){
#if defined(__WIN32__) || defined(__WIN64__)
      err = ERROR_NOT_FOUND;
#else
      err = ENOENT;
#endif
    }
  }
  if( err != 0 )
    newObject<Exception>(err + errorOffset,__PRETTY_FUNCTION__)->throwSP();
  return pObject;
}
//-----------------------------------------------------------------------------
template <
  typename T,
  EmbeddedTreeNode<T> & (*N)(const T &),
  T & (*O) (const EmbeddedTreeNode<T> &,T *),
  intptr_t (*C)(const T &,const T &)
> inline
void EmbeddedTree<T,N,O,C>::rotate(EmbeddedTreeNode<T> ** pNode,intptr_t c)
{
  uintptr_t i = c2i(c), j = 1u - i;
  EmbeddedTreeNode<T> * node = *pNode;
  *pNode = node->leafs_[i];
  node->leafs_[i] = (*pNode)->leafs_[j];
  (*pNode)->leafs_[j] = node;
}
//-----------------------------------------------------------------------------
template <
  typename T,
  EmbeddedTreeNode<T> & (*N)(const T &),
  T & (*O) (const EmbeddedTreeNode<T> &,T *),
  intptr_t (*C)(const T &,const T &)
> inline
EmbeddedTree<T,N,O,C> & EmbeddedTree<T,N,O,C>::insert(const T & object,bool throwIfExist,bool deleteIfExist,T ** pObject)
{
  Stack stack;
  T * obj = internalFind(&root_,object,throwIfExist,false,deleteIfExist,false,&stack);
  if( obj == NULL ){
    N(object).left_ = NULL;
    N(object).right_ = NULL;
    N(object).leaf_ = 0;
    if( stack.sp_ > 0 ){
      stack.sp_--;
      (*stack.node_[stack.sp_])->leafs_[c2i(stack.leaf_[stack.sp_])] = &N(object);
      (*stack.node_[stack.sp_])->leaf_ = stack.leaf_[stack.sp_];
      while( --stack.sp_ >= 0 ){
        intptr_t & c = (*stack.node_[stack.sp_])->leaf_;
        if( c == 0 ) continue;
        rotate(stack.node_[stack.sp_],c);
        c = 0;
      }
    }
    else {
      root_ = &N(object);
    }
    count_++;
  }
  else if( deleteIfExist ){
    deleteObject(&object);
  }
  if( pObject != NULL ) *pObject = obj;
  return *this;
}
//-----------------------------------------------------------------------------
template <
  typename T,
  EmbeddedTreeNode<T> & (*N)(const T &),
  T & (*O) (const EmbeddedTreeNode<T> &,T *),
  intptr_t (*C)(const T &,const T &)
> inline
EmbeddedTree<T,N,O,C> & EmbeddedTree<T,N,O,C>::remove(const T & object,bool throwIfNotExist,bool deleteIfNotExist,T ** pObject)
{
  Stack stack;
  T * obj = internalFind(&root_,object,false,throwIfNotExist,false,deleteIfNotExist,&stack);
  if( obj != NULL ){
  }
  else if( deleteIfNotExist ){
    deleteObject(&object);
  }
  if( pObject != NULL ) *pObject = obj;
  return *this;
}
//-----------------------------------------------------------------------------
#ifndef NDEBUG
//-----------------------------------------------------------------------------
/*template <
  typename T,
  EmbeddedTreeNode<T> & (*N)(const T &),
  T & (*O) (const EmbeddedTreeNode<T> &,T *),
  intptr_t (*C)(const T &,const T &)
> inline
EmbeddedTree<T,N,O,C> & EmbeddedTree<T,N,O,C>::saveEmbeddedTreeGraph(AsyncFile & file) const
{
  Array<Array<utf8::String> > aGraph;
  Array<EmbeddedTreeNode *> graph;
  Array<intptr_t> leafs;
  uintptr_t max = 0;
  graph.add(root_);
  leafs.add(-1);
  for(;;){
    if( graph[graph.count() - 1] == NULL ){
      graph.resize(graph.count() - 1);
      leafs.resize(leafs.count() - 1);
    }
    if( graph.count() == 0 ) break;
    graph.add(graph[graph.count() - 1]->leafs_[leafs[leafs.count() - 1]]);
    leafs.add(-1);
    utf8::String node(
      utf8::String("(") +
      O(*graph[graph.count() - 1],NULL) + "," +
      utf8::int2Str(graph[graph.count() - 1]->leaf_) + ")"
    );
  }
    if( pGraph->parent_ == NULL || pGraph->parent_->isLeafs() ){
      if( pGraph->level_ >= pGraph->graph_.count() )
        pGraph->graph_.resize(pGraph->level_ + 1);
      pGraph->graph_[pGraph->level_].add(node);
      uintptr_t size = node.size();
      if( size > pGraph->root_->max_ ) pGraph->root_->max_ = size;
    }
    if( pGraph->level_ == 0 ){
      uintptr_t cells = pGraph->graph_[pGraph->graph_.count() - 1].count();
      uintptr_t cellPerNode = uintptr_t(1) << (cells - 1);
      uintptr_t size = pGraph->max_ * cells;
      for( uintptr_t i = 0; i < pGraph->graph_.count(); i++ ){
        uintptr_t cellSize = pGraph->max_ * cellPerNode;
        utf8::String line;
        line.resize(size + 1);
        line.c_str()[size] = '\n';
        for( uintptr_t j = 0; j < pGraph->graph_[i].count(); j++ ){
          memcpy(
            line.c_str() + cellSize * j + cellSize / 2 - pGraph->graph_[i][j].size() / 2,
            pGraph->graph_[i][j].c_str(),
            pGraph->graph_[i][j].size()
          );
        }
        file.writeBuffer(line.c_str(),size + 1);
        cellPerNode >>= 1;
      }
    }
  }
  return *const_cast<EmbeddedTree<T,N,O,C> *>(this);
}*/
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <typename T>
class RBTree {
  public:
    class Node {
      public:
        Node * left_;         /* left child */
        Node * right_;        /* right child */
        Node * parent_;       /* parent */
        NodeColor color_;     /* node color (BLACK, RED) */
        T data_;
    };
    Node * insertNode(T data);
    void deleteNode(Node * z);
  protected:
    static const Node sentinel_;
    typedef enum { BLACK, RED } NodeColor;
    Node * root_ = NIL;               /* root of Red-Black tree */

    void rotateLeft(Node * x);
    void rotateRight(Node * x);
    void insertFixup(Node * x);
    void deleteFixup(Node * x);
  private:
};
//-----------------------------------------------------------------------------
template <typename T>
const Node RBTree<T>::sentinel_ = { &sentinel_, &sentinel_, 0, BLACK };
//-----------------------------------------------------------------------------
template <typename T> inline
~RBTree<T>::RBTree<T>()
{
}
//-----------------------------------------------------------------------------
template <typename T> inline
RBTree<T>::RBTree<T>() : root_(&sentinel_)
{
}
//-----------------------------------------------------------------------------
template <typename T> inline
void RBTree<T>::rotateLeft(Node * x)
{
/**************************
 *  rotate node x to left *
 **************************/
  Node *y = x->right_;
/* establish x->right link */
  x->right_ = y->left_;
  if( y->left != &sentinel_ ) y->left_->parent_ = x;
/* establish y->parent link */
  if( y != &sentinel_ ) y->parent_ = x->parent_;
  if( x->parent_ ){
    if( x == x->parent_->left_ )
      x->parent_->left_ = y;
    else
      x->parent_->right_ = y;
  }
  else {
    root_ = y;
  }
/* link x and y */
  y->left_ = x;
  if( x != &sentinel_ ) x->parent_ = y;
}
//-----------------------------------------------------------------------------
template <typename T> inline
void RBTree<T>::rotateRight(Node * x)
{
/****************************
 *  rotate node x to right  *
 ****************************/
  Node * y = x->left_;
/* establish x->left link */
  x->left_ = y->right_;
  if( y->right_ != &sentinel_ ) y->right_->parent_ = x;
/* establish y->parent link */
  if( y != &sentinel_ ) y->parent_ = x->parent_;
  if( x->parent_ ){
    if( x == x->parent_->right_ )
      x->parent_->right_ = y;
    else
      x->parent_->left_ = y;
  }
  else {
    root_ = y;
  }
/* link x and y */
  y->right_ = x;
  if( x != &sentinel_ ) x->parent_ = y;
}
//-----------------------------------------------------------------------------
template <typename T> inline
void RBTree<T>::insertFixup(Node * x)
{
/*************************************
 *  maintain Red-Black tree balance  *
 *  after inserting node x           *
 *************************************/
/* check Red-Black properties */
  while( x != root_ && x->parent_->color_ == RED ){
/* we have a violation */
    if( x->parent_ == x->parent_->parent_->left_ ){
      Node * y = x->parent_->parent_->right_;
      if( y->color_ == RED ){
/* uncle is RED */
        x->parent_->color_ = BLACK;
        y->color_ = BLACK;
        x->parent_->parent_->color_ = RED;
        x = x->parent_->parent_;
      }
      else {
/* uncle is BLACK */
        if( x == x->parent_->right_ ){
/* make x a left child */
          x = x->parent_;
          rotateLeft(x);
        }
/* recolor and rotate */
        x->parent_->color = BLACK;
        x->parent_->parent_->color_ = RED;
        rotateRight(x->parent_->parent_);
      }
    }
    else {
/* mirror image of above code */
      Node * y = x->parent_->parent_->left_;
      if( y->color_ == RED ){
/* uncle is RED */
        x->parent_->color_ = BLACK;
        y->color_ = BLACK;
        x->parent_->parent_->color_ = RED;
        x = x->parent_->parent_;
      }
      else {
/* uncle is BLACK */
        if( x == x->parent_->left_ ){
          x = x->parent_;
          rotateRight(x);
        }
        x->parent_->color_ = BLACK;
        x->parent_->parent_->color = RED;
        rotateLeft(x->parent_->parent_);
      }
    }
  }
  root_->color_ = BLACK;
}
//-----------------------------------------------------------------------------
template <typename T> inline
Node * RBTree<T>::insertNode(T data)
{
  Node * current, * parent, * x;
/***********************************************
 *  allocate node for data and insert in tree  *
 ***********************************************/
/* find where node belongs */
  current = root_;
  parent = 0;
  while( current != NIL ){
    if( compEQ(data,current->data) ) return current;
    parent = current;
    current = compLT(data,current->data) ? current->left_ : current->right_;
  }
/* setup new node */
  if ((x = malloc (sizeof(*x))) == 0) {
        printf (&quot;insufficient memory (insertNode)\n&quot;);
        exit(1);
  }
  x->data = data;
  x->parent_ = parent;
  x->left_ = &sentinel_;
  x->right_ = &sentinel_;
  x->color_ = RED;
/* insert node in tree */
  if( parent ){
    if(compLT(data,parent->data_))
      parent->left_ = x;
    else
      parent->right_ = x;
  }
  else {
    root_ = x;
  }
  insertFixup(x);
  return x;
}
//-----------------------------------------------------------------------------
template <typename T> inline
void RBTree<T>::deleteFixup(Node * x)
{
/*************************************
 *  maintain Red-Black tree balance  *
 *  after deleting node x            *
 *************************************/
  while( x != root_ && x->color_ == BLACK ){
    if( x == x->parent_->left_ ){
      Node * w = x->parent_->right_;
      if( w->color_ == RED ){
        w->color_ = BLACK;
        x->parent_->color_ = RED;
        rotateLeft(x->parent_);
        w = x->parent_->right_;
      }
      if( w->left_->color_ == BLACK && w->right_->color_ == BLACK ){
        w->color_ = RED;
        x = x->parent_;
      }
      else {
        if( w->right_->color_ == BLACK ){
          w->left_->color_ = BLACK;
          w->color_ = RED;
          rotateRight (w);
          w = x->parent_->right_;
        }
        w->color_ = x->parent_->color_;
        x->parent_->color_ = BLACK;
        w->right_->color_ = BLACK;
        rotateLeft(x->parent_);
        x = root_;
      }
    }
    else {
      Node * w = x->parent_->left_;
      if( w->color_ == RED ){
        w->color_ = BLACK;
        x->parent_->color_ = RED;
        rotateRight(x->parent_);
        w = x->parent_->left_;
      }
      if( w->right_->color_ == BLACK && w->left_->color_ == BLACK ){
        w->color_ = RED;
        x = x->parent_;
      }
      else {
        if( w->left_->color_ == BLACK ){
          w->right_->color_ = BLACK;
          w->color_ = RED;
          rotateLeft(w);
          w = x->parent_->left_;
        }
        w->color_ = x->parent_->color_;
        x->parent_->color_ = BLACK;
        w->left_->color_ = BLACK;
        rotateRight(x->parent_);
        x = root_;
      }
    }
  }
  x->color = BLACK;
}
//-----------------------------------------------------------------------------
template <typename T> inline
void RBTree<T>::deleteNode(Node * z)
{
  Node * x, * y;
/*****************************
 *  delete node z from tree  *
 *****************************/
  if( !z || z == &sentinel_ ) return;
  if( z->left_ == &sentinel_ || z->right_ == &sentinel_ ){
/* y has a NIL node as a child */
    y = z;
  }
  else {
/* find tree successor with a NIL node as a child */
    y = z->right_;
    while( y->left_ != &sentinel_ ) y = y->left_;
  }
/* x is y's only child */
  if( y->left_ != &sentinel_ ) x = y->left_; else x = y->right_;
/* remove y from the parent chain */
  x->parent_ = y->parent_;
  if( y->parent_ ){
    if( y == y->parent_->left_ )
      y->parent_->left_ = x;
    else
      y->parent_->right_ = x;
  }
  else {
    root_ = x;
  }
  if( y != z ){
    y->left_->parent_ = z;
    y->right_->parent_ = z;
    if( y->parent_->left_ == y ) y->parent_->left_ = z; else y->parent_->right_ = z;
//    z->data_ = y->data_;
  }
  if( y->color_ == BLACK ) deleteFixup(x);
  free(y);
}
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
typedef int T;                  /* type of item to be stored */
#define compLT(a,b) (a < b)
#define compEQ(a,b) (a == b)

/* Red-Black tree description */
typedef enum { BLACK, RED } nodeColor;

typedef struct Node_ {
    struct Node_ *left;         /* left child */
    struct Node_ *right;        /* right child */
    struct Node_ *parent;       /* parent */
    nodeColor color;            /* node color (BLACK, RED) */
    T data;                     /* data stored in node */
} Node;

#define NIL &sentinel           /* all leafs are sentinels */
Node sentinel = { NIL, NIL, 0, BLACK, 0};

Node *root = NIL;               /* root of Red-Black tree */

void rotateLeft(Node *x) {

   /**************************
    *  rotate node x to left *
    **************************/

    Node *y = x->right;

    /* establish x->right link */
    x->right = y->left;
    if (y->left != NIL) y->left->parent = x;

    /* establish y->parent link */
    if (y != NIL) y->parent = x->parent;
    if (x->parent) {
        if (x == x->parent->left)
            x->parent->left = y;
        else
            x->parent->right = y;
    } else {
        root = y;
    }

    /* link x and y */
    y->left = x;
    if (x != NIL) x->parent = y;
}

void rotateRight(Node *x) {

   /****************************
    *  rotate node x to right  *
    ****************************/

    Node *y = x->left;

    /* establish x->left link */
    x->left = y->right;
    if (y->right != NIL) y->right->parent = x;

    /* establish y->parent link */
    if (y != NIL) y->parent = x->parent;
    if (x->parent) {
        if (x == x->parent->right)
            x->parent->right = y;
        else
            x->parent->left = y;
    } else {
        root = y;
    }

    /* link x and y */
    y->right = x;
    if (x != NIL) x->parent = y;
}

void insertFixup(Node *x) {

   /*************************************
    *  maintain Red-Black tree balance  *
    *  after inserting node x           *
    *************************************/

    /* check Red-Black properties */
    while (x != root && x->parent->color == RED) {
        /* we have a violation */
        if (x->parent == x->parent->parent->left) {
            Node *y = x->parent->parent->right;
            if (y->color == RED) {

                /* uncle is RED */
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            } else {

                /* uncle is BLACK */
                if (x == x->parent->right) {
                    /* make x a left child */
                    x = x->parent;
                    rotateLeft(x);
                }

                /* recolor and rotate */
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rotateRight(x->parent->parent);
            }
        } else {

            /* mirror image of above code */
            Node *y = x->parent->parent->left;
            if (y->color == RED) {

                /* uncle is RED */
                x->parent->color = BLACK;
                y->color = BLACK;
                x->parent->parent->color = RED;
                x = x->parent->parent;
            } else {

                /* uncle is BLACK */
                if (x == x->parent->left) {
                    x = x->parent;
                    rotateRight(x);
                }
                x->parent->color = BLACK;
                x->parent->parent->color = RED;
                rotateLeft(x->parent->parent);
            }
        }
    }
    root->color = BLACK;
}

Node *insertNode(T data) {
    Node *current, *parent, *x;

   /***********************************************
    *  allocate node for data and insert in tree  *
    ***********************************************/

    /* find where node belongs */
    current = root;
    parent = 0;
    while (current != NIL) {
        if (compEQ(data, current->data)) return (current);
        parent = current;
        current = compLT(data, current->data) ?
            current->left : current->right;
    }

    /* setup new node */
    if ((x = malloc (sizeof(*x))) == 0) {
        printf (&quot;insufficient memory (insertNode)\n&quot;);
        exit(1);
    }
    x->data = data;
    x->parent = parent;
    x->left = NIL;
    x->right = NIL;
    x->color = RED;

    /* insert node in tree */
    if(parent) {
        if(compLT(data, parent->data))
            parent->left = x;
        else
            parent->right = x;
    } else {
        root = x;
    }

    insertFixup(x);
    return(x);
}

void deleteFixup(Node *x) {

   /*************************************
    *  maintain Red-Black tree balance  *
    *  after deleting node x            *
    *************************************/

    while (x != root && x->color == BLACK) {
        if (x == x->parent->left) {
            Node *w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateLeft (x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rotateRight (w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rotateLeft (x->parent);
                x = root;
            }
        } else {
            Node *w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotateRight (x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    rotateLeft (w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rotateRight (x->parent);
                x = root;
            }
        }
    }
    x->color = BLACK;
}

void deleteNode(Node *z) {
    Node *x, *y;

   /*****************************
    *  delete node z from tree  *
    *****************************/

    if (!z || z == NIL) return;


    if (z->left == NIL || z->right == NIL) {
        /* y has a NIL node as a child */
        y = z;
    } else {
        /* find tree successor with a NIL node as a child */
        y = z->right;
        while (y->left != NIL) y = y->left;
    }

    /* x is y's only child */
    if (y->left != NIL)
        x = y->left;
    else
        x = y->right;

    /* remove y from the parent chain */
    x->parent = y->parent;
    if (y->parent)
        if (y == y->parent->left)
            y->parent->left = x;
        else
            y->parent->right = x;
    else
        root = x;

    if (y != z) z->data = y->data;


    if (y->color == BLACK)
        deleteFixup (x);

    free (y);
}

Node *findNode(T data) {

   /*******************************
    *  find node containing data  *
    *******************************/

    Node *current = root;
    while(current != NIL)
        if(compEQ(data, current->data))
            return (current);
        else
            current = compLT (data, current->data) ?
                current->left : current->right;
    return(0);
}

void main(int argc, char **argv) {
    int a, maxnum, ct;
    Node *t;

    /* command-line:
     *
     *   rbt maxnum
     *
     *   rbt 2000
     *       process 2000 records
     *
     */

    maxnum = atoi(argv[1]);

    for (ct = maxnum; ct; ct--) {
        a = rand() % 9 + 1;
        if ((t = findNode(a)) != NULL) {
            deleteNode(t);
        } else {
            insertNode(a);
        }
    }
}
//-----------------------------------------------------------------------------
} // namespace ksys
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
