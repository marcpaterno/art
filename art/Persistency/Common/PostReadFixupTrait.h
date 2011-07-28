#ifndef art_Persistency_Common_PostReadFixupTrait_h
#define art_Persistency_Common_PostReadFixupTrait_h

namespace art {
  namespace helpers {
    struct DoNoPostReadFixup {
      void touch() { }
      template<typename C>
      void operator()(const C &) const { }
    };

    struct PostReadFixup {
      PostReadFixup() : fixed_(false) { }
      void touch() { fixed_ = false; }
      template<typename C>
      void operator()(const C & c) const {
        if (!fixed_) {
          fixed_ = true;
          for (typename C::const_iterator i = c.begin(), e = c.end(); i != e; ++i)
            (*i)->fixup();
        }
      }
    private:
      mutable bool fixed_;
    };

    template<typename T>
    struct PostReadFixupTrait {
     typedef DoNoPostReadFixup type;
    };
  }
}

#endif /* art_Persistency_Common_PostReadFixupTrait_h */

// Local Variables:
// mode: c++
// End:
