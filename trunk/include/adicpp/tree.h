/*-
 * Copyright 2007-2008 Guram Dukashvili
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
class AsyncFile;
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
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#elif defined(__BCPLUSPLUS__)
#pragma option push -a1
#endif
class PACKED RBTreeNode { // base class for deriving
  public:
    RBTreeNode * left_;         /* left child */
    RBTreeNode * right_;        /* right child */
    RBTreeNode * parent_;       /* parent */
    uint8_t color_;             /* node color (BLACK, RED) */
    
    template <typename OT> inline
    OT & object(const RBTreeNode & node) const {
      return *(OT *) const_cast<uint8_t *>((const uint8_t *) this - uintptr_t(&node));
    }
};
#ifdef _MSC_VER
#pragma pack(pop)
#elif defined(__BCPLUSPLUS__)
#pragma option pop
#endif
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class RBTreeBenchmarkObject {
  public:
    intmax_t key_;
      
    static RBTreeNode & treeO2N(const RBTreeBenchmarkObject & object,uintptr_t *){
      return object.treeNode_;
    }
    static RBTreeBenchmarkObject & treeN2O(const RBTreeNode & node,uintptr_t *){
      return node.object<RBTreeBenchmarkObject>(reinterpret_cast<RBTreeBenchmarkObject *>(NULL)->treeNode_);
    }
    static intptr_t treeCO(const RBTreeBenchmarkObject & a0,const RBTreeBenchmarkObject & a1,uintptr_t *){
      return a0.key_ > a1.key_ ? 1 : a0.key_ < a1.key_ ? -1 : 0;
    }
    mutable RBTreeNode treeNode_;
};
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
template <
  typename OT,
  typename PT,
  RBTreeNode & O2N(const OT &,PT *),
  OT & N2O(const RBTreeNode &,PT *),
  intptr_t CO(const OT &,const OT &,PT *)
>
class RBTree {
  // member methods declared inside class for borland (bugland) compiler compatibility
  public:
    ~RBTree() {}
    RBTree() : param_(NULL)
    {
      sentinel_.left_ = &sentinel_;
      sentinel_.right_ = &sentinel_;
      sentinel_.parent_ = NULL;
      sentinel_.color_ = BLACK;
      clear();
    }

    OT * find(const OT & object) const
    {
      RBTreeNode * current = root_->left_;
      for(;;){
        if( current == &sentinel_ ){ current = NULL; break; }
        intptr_t c = CO(object,N2O(*current,param_),param_);
        if( c == 0 ) break;
        current = c < 0 ? current->left_ : current->right_;
      }
      return current == NULL ? NULL : &N2O(*current,param_);
    }

    RBTree<OT,PT,O2N,N2O,CO> & insert(const OT & object,bool throwIfExist = true,bool deleteIfExist = true,OT ** pObject = NULL)
    {
      RBTreeNode * y, * x = &O2N(object,param_);
      if( (y = treeInsertHelp(x)) != NULL ){
        if( deleteIfExist ) deleteObject(&object);
        if( pObject != NULL ) *pObject = &N2O(*y,param_);
        if( throwIfExist ) newObjectV1C2<Exception>(
#if defined(__WIN32__) || defined(__WIN64__)
          ERROR_ALREADY_EXISTS + errorOffset
#else
          EEXIST
#endif
          ,__PRETTY_FUNCTION__
        )->throwSP();
        return *this;
      }
      if( pObject != NULL ) *pObject = const_cast<OT *>(&object);
      x->color_ = RED;
      while( x->parent_->color_ == RED ){
        if( x->parent_ == x->parent_->parent_->left_ ){
          y = x->parent_->parent_->right_;
          if( y->color_ == RED ){
    	      x->parent_->color_ = BLACK;
	          y->color_ = BLACK;
	          x->parent_->parent_->color_ = RED;
	          x = x->parent_->parent_;
          }
          else {
	          if( x == x->parent_->right_){
	            x = x->parent_;
	            rotateLeft(x);
	          }
	          x->parent_->color_ = BLACK;
	          x->parent_->parent_->color_ = RED;
	          rotateRight(x->parent_->parent_);
          } 
        }
        else {
          y = x->parent_->parent_->left_;
          if( y->color_ == RED ){
	          x->parent_->color_ = BLACK;
	          y->color_ = BLACK;
	          x->parent_->parent_->color_ = RED;
	          x = x->parent_->parent_;
          }
          else {
	          if( x == x->parent_->left_ ){
	            x = x->parent_;
	            rotateRight(x);
	          }
	          x->parent_->color_ = BLACK;
	          x->parent_->parent_->color_ = RED;
	          rotateLeft(x->parent_->parent_);
          } 
        }
      }
      root_->left_->color_ = BLACK;
      count_++;
      return *this;
    }

    RBTree<OT,PT,O2N,N2O,CO> & remove(const OT & object)
    {
      RBTreeNode * z = &O2N(object,param_), * x, * y;
      y = ((z->left_ == &sentinel_) || (z->right_ == &sentinel_)) ? z : getSuccessorOf(z);
      x = (y->left_ == &sentinel_) ? y->right_ : y->left_;
/* x is y's only child */
      if( y->left_ != &sentinel_ ) x = y->left_; else x = y->right_;
/* assignment of y->p to x->p is intentional */  
      if( root_ == (x->parent_ = y->parent_) ){
        root_->left_ = x;
      }
      else {
        if( y == y->parent_->left_ ){
          y->parent_->left_ = x;
        }
        else {
          y->parent_->right_ = x;
        }
      }
      if( y != z ){
        y->left_ = z->left_;
        y->right_ = z->right_;
        y->parent_ = z->parent_;
        z->left_->parent_ = z->right_->parent_ = y;
        if( z == z->parent_->left_ ){
          z->parent_->left_ = y;
        }
        else {
          z->parent_->right_ = y;
        }
        if( y->color_ != RED ){
          y->color_ = z->color_;
          removeFixup(x);
        }
        else {
          y->color_ = z->color_;
        }
      }
      else if( y->color_ != RED ){
        removeFixup(x);
      }
      z->parent_ = z->right_ = z->left_ = NULL;
      count_--;
      return *this;
    }

    RBTree<OT,PT,O2N,N2O,CO> & replace(const OT & oldObject,const OT & newObject)
    {
      RBTreeNode * oldNode = &O2N(oldObject,param_);
      RBTreeNode * newNode = &O2N(newObject,param_);
      newNode->parent_ = oldNode->parent_;
      newNode->left_ = oldNode->left_;
      newNode->right_ = oldNode->right_;
      newNode->color_ = oldNode->color_;
      if( root_->left_ == oldNode ) root_->left_ = newNode;
      if( oldNode->parent_->left_ == oldNode ){
        oldNode->parent_->left_ = newNode;
      }
      else {
        oldNode->parent_->right_ = newNode;
      }
      if( oldNode->left_ != &sentinel_ ) oldNode->left_->parent_ = newNode;
      if( oldNode->right_ != &sentinel_ ) oldNode->right_->parent_ = newNode;
      return *this;
    }

    RBTree<OT,PT,O2N,N2O,CO> & drop(const OT & object)
    {
      remove(object);
      deleteObject(&object);
      return *this;
    }

    RBTree<OT,PT,O2N,N2O,CO> & clear()
    {
      root_ = &rootNode_;
      rootNode_.left_ = &sentinel_;
      rootNode_.right_ = &sentinel_;
      rootNode_.parent_ = NULL;
      rootNode_.color_ = BLACK;
      count_ = 0;
      return *this;
    }

    RBTree<OT,PT,O2N,N2O,CO> & drop()
    {
      if( count_ > 0 ){
        Walker walker(*this);
        while( walker.next() ) deleteObject(&walker.object());
        clear();
      }
      return *this;
    }

    RBTree<OT,PT,O2N,N2O,CO> & param(PT * param){ param_ = param; return *this; }

/*    template <typename KT> OT * find(const KT & key) const {
      RBTreeNode * current = root_;
      for(;;){
        if( current == &sentinel_ ){ current = NULL; break; }
        intptr_t c = CK(N2O(*current),key);
        if( c == 0 ) break;
        current = c > 0 ? current->left_ : current->right_;
      }
      return current == NULL ? NULL : &N2O(*current);
    }
    template <typename KT> OT & remove(const KT & key,bool throwIfNotExist = true){
      OT * pObject = find(key);
      if( pObject == NULL ){
        if( throwIfNotExist ) newObjectV1C2<Exception>(
#if defined(__WIN32__) || defined(__WIN64__)
          ERROR_NOT_FOUND + errorOffset
#else
          ENOENT
#endif
          ,__PRETTY_FUNCTION__
        )->throwSP();
      }
      else {
        remove(*pObject);
      }
      return *pObject;
    }*/
    class Walker {
      public:
        Walker(const RBTree<OT,PT,O2N,N2O,CO> & tree) : tree_(tree), sp_(0) {
          path_[0].node_ = tree_.root_;
          path_[0].left_ = tree_.root_->left_;
          path_[0].right_ = tree_.root_->right_;
          path_[0].pointer_ = -1;
        }
        OT & object() const { return N2O(*path_[sp_].node_,tree_.param_); }
        bool next(){
          for(;;){
            if( path_[sp_].pointer_ < 0 ){
              path_[sp_].pointer_++;
              if( path_[sp_].left_ != &tree_.sentinel_ ){
                path_[sp_ + 1].node_ = path_[sp_].left_;
                path_[sp_ + 1].left_ = path_[sp_].left_->left_;
                path_[sp_ + 1].right_ = path_[sp_].left_->right_;
                path_[sp_ + 1].pointer_ = -1;
                sp_++;
              }
            }
            else if( path_[sp_].pointer_ > 0 ){
              path_[sp_].pointer_++;
              if( path_[sp_].pointer_ > 2 || path_[sp_].right_ == &tree_.sentinel_ ){
                sp_--;
              }
              else {
                path_[sp_ + 1].node_ = path_[sp_].right_;
                path_[sp_ + 1].left_ = path_[sp_].right_->left_;
                path_[sp_ + 1].right_ = path_[sp_].right_->right_;
                path_[sp_ + 1].pointer_ = -1;
                sp_++;
              }
            }
            else {
              path_[sp_].pointer_++;
              break;
            }
          }
          return sp_ > 0;
        }
      protected:
        class Node {
          public:
            RBTreeNode * node_;
            RBTreeNode * left_;
            RBTreeNode * right_;
            intptr_t pointer_;
        };
        const RBTree<OT,PT,O2N,N2O,CO> & tree_;
        Node path_[sizeof(uintptr_t) * 8 + 2];
        intptr_t sp_;
    };
    friend class Walker;

    void benchmark(uintptr_t elCount,uintptr_t cycles = 1)
    {
      Array<RBTreeBenchmarkObject> nodes;
      nodes.resize(elCount);
      Array<RBTreeBenchmarkObject *> pNodes;
      pNodes.resize(elCount);
      Randomizer rnd;

//  assert( root_ == &rootNode_ );

      uint64_t seqInsTime = 0, seqFindTime = 0, seqRemTime = 0;
      uint64_t rndInsTime = 0, rndFindTime = 0, rndRemTime = 0;

#if HAVE_NICE
      nice(-20);
#endif
      for( intptr_t cycle = cycles - 1; cycle >= 0; cycle-- ){
        uint64_t t = gettimeofday();
        for( intptr_t i = elCount - 1; i >= 0; i-- ){
          nodes[i].key_ = i + 1;
          insert(nodes[i]);
        }
        seqInsTime += gettimeofday() - t;
        t = gettimeofday();
        for( intptr_t i = elCount - 1; i >= 0; i-- )
          if( find(nodes[i]) == NULL ) fprintf(stderr,"seq find failed %"PRIdPTR"\n",i);
        seqFindTime += gettimeofday() - t;
        t = gettimeofday();
        for( intptr_t i = elCount - 1; i >= 0; i-- ) remove(nodes[i]);
        seqRemTime += gettimeofday() - t;

//    assert( root_ == &rootNode_ );

        rnd.srand(cycle);
        for( intptr_t i = elCount - 1; i >= 0; i-- ) pNodes[i] = &nodes[i];
        for( intptr_t i = elCount - 1; i >= 0; i-- ){
          ksys::xchg(
            pNodes[uintptr_t(rnd.random() % elCount)],
            pNodes[uintptr_t(rnd.random() % elCount)]
          );
        }
        t = gettimeofday();
        for( intptr_t i = elCount - 1; i >= 0; i-- ) insert(*pNodes[i]);
        rndInsTime += gettimeofday() - t;
        t = gettimeofday();
        for( intptr_t i = elCount - 1; i >= 0; i-- )
          if( find(*pNodes[i]) == NULL ) fprintf(stderr,"rnd find failed %"PRIdPTR"\n",i);
        rndFindTime += gettimeofday() - t;
        t = gettimeofday();
        for( intptr_t i = elCount - 1; i >= 0; i-- ) remove(*pNodes[i]);
        rndRemTime += gettimeofday() - t;

//    assert( root_ == &rootNode_ );
      }

      seqInsTime /= cycles;
      seqFindTime /= cycles;
      seqRemTime /= cycles;
      rndInsTime /= cycles;
      rndFindTime /= cycles;
      rndRemTime /= cycles;
  
      fprintf(stderr,"seq inserts: %8"PRIu64".%04"PRIu64" ips, ellapsed %s\n",
        uint64_t(elCount) * 1000000u / seqInsTime,
        uint64_t(elCount) * 10000u * 1000000u / seqInsTime -
        uint64_t(elCount) * 1000000u / seqInsTime * 10000u,
        (const char *) utf8::elapsedTime2Str(seqInsTime).getOEMString()
      );
      fprintf(stderr,"  seq finds: %8"PRIu64".%04"PRIu64" fps, ellapsed %s\n",
        uint64_t(elCount) * 1000000u / seqFindTime,
        uint64_t(elCount) * 10000u * 1000000u / seqFindTime -
        uint64_t(elCount) * 1000000u / seqFindTime * 10000u,
        (const char *) utf8::elapsedTime2Str(seqFindTime).getOEMString()
      );
      fprintf(stderr,"seq removes: %8"PRIu64".%04"PRIu64" rps, ellapsed %s\n",
        uint64_t(elCount) * 1000000u / seqRemTime,
        uint64_t(elCount) * 10000u * 1000000u / seqRemTime -
        uint64_t(elCount) * 1000000u / seqRemTime * 10000u,
        (const char *) utf8::elapsedTime2Str(seqRemTime).getOEMString()
      );

      fprintf(stderr,"rnd inserts: %8"PRIu64".%04"PRIu64" ips, ellapsed %s\n",
        uint64_t(elCount) * 1000000u / rndInsTime,
        uint64_t(elCount) * 10000u * 1000000u / rndInsTime -
        uint64_t(elCount) * 1000000u / rndInsTime * 10000u,
        (const char *) utf8::elapsedTime2Str(rndInsTime).getOEMString()
      );
      fprintf(stderr,"  rnd finds: %8"PRIu64".%04"PRIu64" fps, ellapsed %s\n",
        uint64_t(elCount) * 1000000u / rndFindTime,
        uint64_t(elCount) * 10000u * 1000000u / rndFindTime -
        uint64_t(elCount) * 1000000u / rndFindTime * 10000u,
        (const char *) utf8::elapsedTime2Str(rndFindTime).getOEMString()
      );
      fprintf(stderr,"rnd removes: %8"PRIu64".%04"PRIu64" rps, ellapsed %s\n",
        uint64_t(elCount) * 1000000u / rndRemTime,
        uint64_t(elCount) * 10000u * 1000000u / rndRemTime -
        uint64_t(elCount) * 1000000u / rndRemTime * 10000u,
        (const char *) utf8::elapsedTime2Str(rndRemTime).getOEMString()
      );

      uint64_t index = uint64_t(elCount) * 10000u * 1000000u;
      index /= (seqInsTime + seqFindTime + seqRemTime + rndInsTime + rndFindTime + rndRemTime) / 6;
      index /= 100000u;

      fprintf(stderr,"overall index: %"PRIu64".%04"PRIu64"\n",index / 10000u,index - (index / 10000u) * 10000u);
    }
  protected:
    typedef enum { BLACK, RED } RBTreeNodeColor;

    mutable RBTreeNode sentinel_;
    mutable RBTreeNode rootNode_;
    mutable RBTreeNode * root_;
    uintptr_t count_;
    PT * param_;

    void rotateLeft(RBTreeNode * x)
    {
      RBTreeNode * y = x->right_;
      x->right_ = y->left_;
      if( y->left_ != &sentinel_ ) y->left_->parent_ = x;
      y->parent_ = x->parent_;
      if( x == x->parent_->left_ ){
        x->parent_->left_ = y;
      }
      else {
        x->parent_->right_ = y;
      }
      y->left_ = x;
      x->parent_ = y;
    }

    void rotateRight(RBTreeNode * y)
    {
      RBTreeNode * x = y->left_;
      y->left_ = x->right_;
      if( x->right_ != &sentinel_ ) x->right_->parent_ = y;
      x->parent_ = y->parent_;
      if( y == y->parent_->left_ ){
        y->parent_->left_ = x;
      }
      else {
        y->parent_->right_ = x;
      }
      x->right_ = y;
      y->parent_ = x;
    }

    void insertFixup(RBTreeNode * x)
    {
/* check Red-Black properties */
      while( x != root_ && x->parent_->color_ == RED ){
/* we have a violation */
        if( x->parent_ == x->parent_->parent_->left_ ){
          RBTreeNode * y = x->parent_->parent_->right_;
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
            x->parent_->color_ = BLACK;
            x->parent_->parent_->color_ = RED;
            rotateRight(x->parent_->parent_);
          }
        }
        else {
/* mirror image of above code */
          RBTreeNode * y = x->parent_->parent_->left_;
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
            x->parent_->parent_->color_ = RED;
            rotateLeft(x->parent_->parent_);
          }
        }
      }
      root_->color_ = BLACK;
    }

    void removeFixup(RBTreeNode * x)
    {
      RBTreeNode * w, * rootLeft = root_->left_;

      while( x->color_ != RED && rootLeft != x ){
        if( x == x->parent_->left_ ){
          w = x->parent_->right_;
          if( w->color_ == RED ){
	          w->color_ = BLACK;
	          x->parent_->color_ = RED;
	          rotateLeft(x->parent_);
	          w = x->parent_->right_;
          }
          if( w->right_->color_ != RED && w->left_->color_ != RED ){ 
	          w->color_ = RED;
	          x = x->parent_;
          }
          else {
	          if( w->right_->color_ != RED ){
	            w->left_->color_ = BLACK;
	            w->color_ = RED;
	            rotateRight(w);
	            w = x->parent_->right_;
	          }
	          w->color_ = x->parent_->color_;
	          x->parent_->color_ = BLACK;
	          w->right_->color_ = BLACK;
	          rotateLeft(x->parent_);
	          x = rootLeft; /* this is to exit while loop */
          }
        }
        else { /* the code below is has left and right switched from above */
          w = x->parent_->left_;
          if( w->color_ == RED ){
	          w->color_ = BLACK;
	          x->parent_->color_ = RED;
	          rotateRight(x->parent_);
	          w = x->parent_->left_;
          }
          if( w->right_->color_ != RED && w->left_->color_ != RED ){ 
	          w->color_ = RED;
	          x = x->parent_;
          }
          else {
	          if( w->left_->color_ != RED ){
	            w->right_->color_ = BLACK;
	            w->color_ = RED;
	            rotateLeft(w);
	            w = x->parent_->left_;
	          }
	          w->color_ = x->parent_->color_;
	          x->parent_->color_ = BLACK;
	          w->left_->color_ = BLACK;
	          rotateRight(x->parent_);
	          x = rootLeft; /* this is to exit while loop */
          }
        }
      }
      x->color_ = BLACK;
    }

    RBTreeNode * getSuccessorOf(RBTreeNode * x) const
    {
      RBTreeNode * y;
/* assignment to y is intentional */  
      if( (y = x->right_) != &sentinel_ ){
/* returns the minium of the right subtree of x */  
        while( y->left_ != &sentinel_ ) y = y->left_;
        return y;
      }
      y = x->parent_;
/* sentinel used instead of checking for nil */    
      while( x == y->right_ ){
        x = y;
        y = y->parent_;
      }
      if( y == root_ ) return &sentinel_;
      return y;
    }

    RBTreeNode * treeInsertHelp(RBTreeNode * z)
    {
      RBTreeNode * y = root_, * x = root_->left_;
      z->left_ = z->right_ = &sentinel_;
      intptr_t c = 0;
      while( x != &sentinel_ ){
        y = x;
        c = CO(N2O(*z,param_),N2O(*x,param_),param_);
        if( c < 0 ){
          x = x->left_;
        }
        else if( c > 0 ){
          x = x->right_;
        }
        else
          return x;
      }
      z->parent_ = y;
      if( y == root_ || c < 0 ){ 
        y->left_ = z;
      }
      else {
        y->right_ = z;
      }
      return NULL;
    }
  private:
    RBTree(const RBTree<OT,PT,O2N,N2O,CO> &);
    void operator = (const RBTree<OT,PT,O2N,N2O,CO> &);
};
//-----------------------------------------------------------------------------
typedef
  RBTree<
    RBTreeBenchmarkObject,
    uintptr_t,
    RBTreeBenchmarkObject::treeO2N,
    RBTreeBenchmarkObject::treeN2O,
    RBTreeBenchmarkObject::treeCO
> RBTreeBenchmarkTree;
//-----------------------------------------------------------------------------
} // namespace ksys
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
