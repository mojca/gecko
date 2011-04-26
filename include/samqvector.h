#ifndef SAMQVECTOR_H
#define SAMQVECTOR_H

#include <QVector>

namespace Sam {
    template<typename T>
    struct vector_traits< QVector<T> > {
        typedef T value_type;
        typedef QVector< QVector<T> > vecvec_type;

        static void do_reserve (QVector<T> & v, unsigned int n) {
            v.reserve (n);
        }

        template<typename V>
        static void do_fill (QVector<T> & v, unsigned int length, V val) {
            v.fill(val, length);
        }
    };
};

#endif // SAMQVECTOR_H
