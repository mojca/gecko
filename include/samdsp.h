/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SAMDSP_H
#define SAMDSP_H

#define TIME 0
#define AMP 1
#define PHASE 1

#include <cmath>
#include <vector>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_trig.h>

namespace Sam {
    template<typename T>
    struct vector_traits;

    template<typename T>
    struct vector_traits< std::vector<T> > {
        typedef T value_type;
        typedef std::vector< std::vector<T> > vecvec_type;
        static void do_reserve (std::vector<T> & v, unsigned int n) { v.reserve (n); }

        template<typename V>
        static void do_fill (std::vector<T> & v, unsigned int length, V val) { v.assign(length, val); }
    };

    // Helper function to get reverse iterators
    template<typename Iter>
    std::reverse_iterator<Iter> rev_iter (Iter x)
    {
        return std::reverse_iterator<Iter> (x);
    }

};

class SamDSP
{
public:

    // Vector actions
    // Returns [time,maximum]
    template<typename T> T max(const T & v)
    {
        using namespace Sam;
        if(v.empty ()) return T ();

        T result;
        typename Sam::vector_traits<T>::value_type max = v.at(0);
        typename Sam::vector_traits<T>::value_type time = 0;

        for(int i = 0; i < static_cast<int> (v.size()); i++)
        {
            if(v[i] > max)
            {
                max  = v[i];
                time = i;
            }
        }
        result.push_back(time);
        result.push_back(max);

        return result;
    }

    // Returns [time,minimum]
    template<typename T> T min(const T & v)
    {
        if(v.empty ()) return T ();

        T result;
        typename Sam::vector_traits<T>::value_type min = v.at(0);
        typename Sam::vector_traits<T>::value_type time = 0;

        for(int i = 0; i < static_cast<int> (v.size()); i++)
        {
            if(v[i] < min)
            {
                min  = v[i];
                time = i;
            }
        }
        result.push_back(time);
        result.push_back(min);

        return result;
    }

    // c_i = a_i + b_i
    template<typename T> T add(const T & a, const T & b)
    {
        T result;
        if(a.size() != b.size())
        {
            fprintf(stderr,"ERROR in SamDSP::add: Size must be equal\n");
            fflush(stderr);
            return result;
        }
        Sam::vector_traits<T>::do_reserve (result, a.size ());

        for(unsigned int i = 0; i < static_cast<unsigned> (a.size()); i++)
        {
                result.push_back(a.at(i) + b.at(i));
        }

        return result;
    }

    // c_i = a_i + b
    template<typename T, typename V> T addC(const T & a, V b)
    {
        T result;

        for(unsigned int i = 0; i < a.size(); i++)
        {
                result.push_back(a.at(i) + b);
        }

        return result;
    }

    // c_i = a_i - b_i
    template<typename T> T sub(const T & a,const T & b)
    {
        T result;
        if(a.size() != b.size())
        {
            fprintf(stderr,"ERROR in SamDSP::sub: Size must be equal\n");
            fflush(stderr);
            return result;
        }

        for(unsigned int i = 0; i < a.size(); i++)
        {
                result.push_back(a.at(i) - b.at(i));
        }

        return result;
    }

    // c_i = a_i - b_j starting from <from>
    template<typename T> T sub(const T & a,const T & b, unsigned int from)
    {
        T result;
        if(from > a.size())
        {
            fprintf(stderr,"ERROR in SamDSP::sub: from is out of range (%d)\n", static_cast<int>(from));
            return result;
        }

        for(unsigned int i = 0; i < a.size(); i++)
        {
                if(i < from || i >= from+b.size())
                    result.push_back(a.at(i));
                else
                    result.push_back(a.at(i) - b.at(i-from));
        }

        return result;
    }

    // c_i = a_i * b_i
    template<typename T> T mul(const T & a,const T & b)
    {
        unsigned int asize = a.size();
        if(asize != b.size())
        {
            fprintf(stderr,"ERROR in SamDSP::mul: Size must be equal\n");
            fflush(stderr);
            return T ();
        }

        T result;
        Sam::vector_traits<T>::do_reserve (result, asize);
        for(unsigned int i = 0; i < asize; i++)
        {
                result.push_back (a.at(i) * b.at(i));
        }

        return result;
    }

    // c_i = a_i / b_i
    template<typename T> T div(const T & a,const T & b)
    {
        unsigned int asize = a.size();
        if(asize != b.size())
        {
            fprintf(stderr,"ERROR in SamDSP::div: Size must be equal\n");
            fflush(stderr);
            return T ();
        }

        T result;
        Sam::vector_traits<T>::do_reserve (result, asize);
        for(unsigned int i = 0; i < asize; i++)
        {
            if(b.at(i) != 0)
            {
                result.push_back (a.at(i) / b.at(i));
            }
            else
            {
                result.push_back (0);
                fprintf(stderr,"WARNING in SamDSP::div: Division by zero encountered\n");
            }
        }

        return result;
    }

    // compares each element of v with the given value.
    // returns a vector containing -1 if the element of v was less, +1 if it was greater
    // and 0 if both were equal (V must be less-than-comparable to the value type of T)
    template<typename T, typename V>
    T compare (const T & v, V value) {
        T result;
        Sam::vector_traits<T>::do_reserve (result, v.size ());

        for (int i = 0; i < static_cast<int> (v.size ()); ++i) {
            if (value < v [i])
                result.push_back (+1);
            else if (v [i] < value)
                result.push_back (-1);
            else
                result.push_back (0);
        }
        return result;
    }

    // returns the sign of each element
    template<typename T>
    T vsign (const T & v) {
        return compare (v, 0);
    }

    double sign (double x) {
        return x < 0 ? -1 : (x > 0 ? +1 : 0);
    }

    // c_i = a_i * factor
    template<typename T, typename V> T scale(const T & a, V factor)
    {
        if(std::isnan(factor))
        {
            fprintf(stderr,"ERROR in SamDSP::scale: Factor is not a number\n");
        }

        T result;
        Sam::vector_traits<T>::do_reserve (result, a.size ());
        for(unsigned int i = 0; i < a.size(); i++)
        {
                if(std::isnan(a.at(i) * factor)) result.push_back(0);
                else result.push_back(a.at(i) * factor);
        }

        return result;
    }

    // c_i = factor / a_i
    template<typename T, typename V> T rscale(const T & a, V factor)
    {
        T result;
        Sam::vector_traits<T>::do_reserve (result, a.size ());

        for(unsigned int i = 0; i< a.size(); i++)
        {
            if(a.at(i) != 0)
            {
                result.push_back(factor / a.at(i));
            }
            else
            {
                result.push_back(0.0);
                fprintf(stderr,"WARNING in SamDSP::rscale: Division by zero encountered\n");
            }
        }

        return result;
    }

    // Shifts vector <distance> elements
    template<typename T> T shift(const T & v, int distance)
    {
        T result(v.size(),0);
        if(abs(distance) > static_cast<int>(v.size()))
        {
            fprintf(stderr,"ERROR in SamDSP::shift: Shift too far\n");
            return v;
        }
        if(distance > 0) std::copy(v.begin(),v.end()-distance,result.begin()+distance);
        else if(distance < 0) std::copy(v.begin()-distance,v.end(),result.begin());  // !! Distance is negative
        else if(distance == 0) result = v;
        return result;
    }

    // Rotates vector <distance> elements (positive values rotate right)
    template<typename T> T rotate(const T & v, int distance)
    {
        if(abs(distance) > static_cast<int>(v.size()))
        {
            fprintf(stderr,"ERROR in SamDSP::rotate: More than 2pi rotated\n");
            return v;
        }
        if (distance == 0)
            return v;

        T result (v);
        typename T::iterator it = result.begin();
        int num_inc = distance > 0 ? v.size () - distance : -distance;
        for (int i = 0; i < num_inc; ++i) ++it;
        std::rotate(result.begin(), it, result.end());
        return result;
    }

    // Pads the vector <left> and <right> with <value>
    template<typename T, typename V> T pad(const T & v, int left, int right, V value)
    {
        T result(v);
        for(int i = 0; i < left; i++)
        {
            result.insert(result.begin(), value);
        }
        for(int i = 0; i < right; i++)
        {
            result.push_back(value);
        }

        return result;
    }

    // Returns from a vector with timestamps and values according to mask
    template<typename T>
    typename Sam::vector_traits<T>::vecvec_type select(const T & v, const T & mask)
    {
        if(v.size() != mask.size())
        {
            //fprintf(stderr,"WARNING in SamDSP::select: Vector sizes do not match (%d vs. %d)\n",static_cast<int>(v.size()),static_cast<int>(mask.size()));
            fflush(stderr);
            exit(1);
        }
        typename Sam::vector_traits<T>::vecvec_type result (2);
        for(int i = 0; i < static_cast<int>(v.size()); i++)
        {
            if(mask.at(i) != 0)
            {
                result[TIME].push_back(i);          // Timestamp at trigger
                result[AMP].push_back(v.at(i));     // Amplitude at trigger
            }
        }

        return result;
    }

    // Returns the average over several windows of v at points in mask, each from left to right
    // It would need as input a trace v and precise timings in mask to average over pulses
    template<typename T> T average(const T & v, const T & mask, unsigned int left, unsigned int right)
    {
        T result (left+right,0);

        if(v.size() != mask.size())
        {
            fprintf(stderr,"WARNING in SamDSP::average: Vector sizes do not match (%d vs. %d)\n",static_cast<int>(v.size()),static_cast<int>(mask.size()));
            fflush(stderr);
            return result;
        }
        if(static_cast<unsigned> (v.size()) <= left+right+1)
        {
            fprintf(stderr,"WARNING in SamDSP::average: Vector is too short (%d vs. %d)\n",static_cast<int>(v.size()),left+right+1);
            fflush(stderr);
            return result;
        }

        unsigned int cnt = 0;
        for(unsigned int i = left; i < mask.size()-right; i++)
        {
            if(mask.at(i) == 1)
            {
                cnt++;
                for(unsigned int k = i-left; k < i+right; k++)
                {
                    result[k-i+left] += v.at(k);
                }
            }
        }

        //cout << "Averaged over " << cnt << " windows" << endl;

        fast_scale(result, 1.0/cnt);
        return result;
    }

    // Returns the average over several windows of v at points in mask, each from left to right
    // It would need as input a trace v and precise timings in mask to average over pulses
    // Also with maximum amount of averaging windows
    template<typename T> T average(const T & v, const T & mask, unsigned int left, unsigned int right, unsigned int max)
    {
        T result(left+right,0);

        if(v.size() != mask.size())
        {
            fprintf(stderr,"WARNING in SamDSP::average: Vector sizes do not match (%d vs. %d)\n",static_cast<int>(v.size()),static_cast<int>(mask.size()));
            fflush(stderr);
            return result;
        }
        if(v.size() <= left+right+1)
        {
            fprintf(stderr,"WARNING in SamDSP::average: Vector is too short (%d vs. %d)\n",static_cast<int>(v.size()),left+right+1);
            fflush(stderr);
            return result;
        }

        unsigned int cnt = 0;
        for(unsigned int i = left; i < mask.size()-right; i++)
        {
            if(mask[i] == 1)
            {
                cnt++;
                for(unsigned int k = i-left; k < i+right; k++)
                {
                    result[k-i+left] += v[k];
                }
                // Break, if maximum is reached
                if(cnt == max) break;
            }
        }

        //std::cout << "Averaged over " << cnt << " windows" << std::endl;

        fast_scale (result, 1.0/cnt);
        return result;
    }

    // Tests if <x> is within <low> and <high>
    template<typename T> bool within(T x, T low, T high)
    {
        return (low <= x) && (x <= high);
    }

    // Returns the euclidean distance of two vectors
    template<typename T> double distance(const T & v, const T & u)
    {
        double distance = 0;

        if(v.size() != u.size())
        {
            fprintf(stderr,"ERROR in SamDSP::distance: Vector sizes do not match (%d vs. %d)\n",static_cast<int>(v.size()),static_cast<int>(u.size()));
            exit(2);
        }

        for(unsigned int i = 0; i < v.size(); i++)
        {
            distance += 1.0*(v.at(i)-u.at(i))*(v.at(i)-u.at(i));
        }
        return sqrt(distance);
    }

    template<typename T> double average(const T & v)
    {
        double avg = 0;
        for(unsigned int i = 0; i < v.size(); i++)
        {
            avg += v.at(i);
        }
        avg /= v.size();
        //cout << "Average: " << avg << endl;
        return avg;
    }

    // Returns scalar product of two vectors
    template<typename T> double scalar(const T & v, const T & u)
    {
        double sum = 0;

        if(v.size() != u.size())
        {
            fprintf(stderr,"ERROR in SamDSP::scalar: Vector sizes do not match (%d vs. %d)\n",static_cast<int>(v.size()),static_cast<int>(u.size()));
            exit(2);
        }

        for(unsigned int i = 0; i < v.size(); i++)
        {
            sum += 1.0*v[i]*u[i];
        }
        return sum;
    }

    // Returns the sum of the vector over the given range
    template<typename T>
    typename Sam::vector_traits<T>::value_type sum(const T & v, unsigned int from, unsigned int to)
    {
        double sum = 0;

        if(to < from || from > to || to > v.size())
        {
            fprintf(stderr,"ERROR in SamDSP::sum: to or from out of range (%d, %d)\n",static_cast<int>(from),static_cast<int>(to));
            exit(2);
        }

        for(unsigned int i = from; i < to; i++)
        {
            sum += v[i];
        }

        return sum;
    }

    // Matrix actions

    // Computes the singularity-tolerant inverse via SVD method
    template<typename T> T gslSvdInverse(const T & m)
    {
        if(m.empty ())
        {
            return T (0);
        }

        typename Sam::vector_traits<T>::value_type tmp = m[0];
        int dm,dn,transform;

        // Check alignment
        if(m.size() >= tmp.size())
        {
            dm = m.size();
            dn = tmp.size();
            transform = 0;
        }
        else
        {
            dm = tmp.size();
            dn = m.size();
            transform = 1;
        }

        tmp.clear();

        // Build working containers
        gsl_matrix *a;
        a = gsl_matrix_calloc(dm,dn);
        for(int i = 0; i < dm; i++)
        {
            for(int j = 0; j < dn; j++)
            {
                if(transform) gsl_matrix_set(a,i,j,m[j][i]);
                else gsl_matrix_set(a,i,j,m[i][j]);
            }
        }
        gsl_matrix *v;
        v = gsl_matrix_calloc(dm,dn);
        gsl_matrix *mi;
        mi = gsl_matrix_calloc(dm,dn);
        gsl_vector *s;
        s = gsl_vector_calloc(dn);
        gsl_vector *w;
        w = gsl_vector_calloc(dn);

        // Decompose
        gsl_linalg_SV_decomp(a,v,s,w);

        // Invert a^-1 = v.s^-1.u^T
        gsl_matrix_transpose(a);
        for(int i = 0; i < dm; i++)
        {
            gsl_vector_view row = gsl_matrix_row(a,i);
            double scale = gsl_vector_get(s,i);
            if(scale < 0.0000001) scale = 0.0;
            if(scale != 0.0)
            {
                gsl_vector_scale(&row.vector,1./scale);
            }
            else
            {
                gsl_vector_set_zero(&row.vector);
                //fprintf(stderr,"Singularity encountered\n");fflush(stderr);
            }
        }
        gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,v,a,0.0,mi);


        //  Build result
        T result;
        Sam::vector_traits<T>::do_reserve (result, dm);
        for(int i = 0; i < dm; i++)
        {
            typename Sam::vector_traits<T>::value_type tmp2(dn);
            Sam::vector_traits<typename T::value_type>::do_reserve (tmp2, dn);
            for(int j = 0; j < dn; j++)
            {
                tmp2.push_back (gsl_matrix_get(mi,i,j));
            }
            result.push_back (tmp2);
        }

        gsl_matrix_free(mi);
        gsl_matrix_free(v);
        gsl_matrix_free(a);
        gsl_vector_free(s);
        gsl_vector_free(w);

        return result;
    }

    template<typename T, typename D>
    typename Sam::vector_traits<T>::vecvec_type delay(const T & v, const D & distance)
    {
        typename Sam::vector_traits<T>::vecvec_type result;
        T tmp;
        for(int i = 0; i < static_cast<int>(distance.size()); i++)
        {
            tmp = shift(v, distance[i]);
            result.push_back(tmp);
        }

        return result;
    }

    // Generates identity matrix of order <size>
    std::vector< std::vector<double> > identity(int size)
    {
        return identity< std::vector< std::vector<double> > > (size);
    }

    template<typename T> T identity(int size)
    {
        T result;
        if(size > 0)
        {
            typename Sam::vector_traits<T>::value_type tmp(size,0);
            tmp[0] = 1;
            std::vector<int> sh = range(0,size,1);
            result = delay(tmp,sh);

            return result;
        }
        else
        {
            fprintf(stderr,"ERROR in SamDSP::identity: Size must be positive integer\n");
            fflush(stderr);
            return result;
        }
    }

    // Kernel generators

    // Generates a bipolar box filter kernel
    std::vector<double> biboxkernel(int width)
    {
        return biboxkernel< std::vector<double> >(width);
    }

    template<typename T> T biboxkernel(int width)
    {
        T result;
        for(int i = 0; i < width; i++)
        {
            result.push_back(1);
        }
        for(int i = 0; i < width; i++)
        {
            result.push_back(-1);
        }
        return result;
    }

    // Generates a box filter kernel
    std::vector<double> boxkernel(int width)
    {
        return boxkernel< std::vector<double> > (width);
    }

    template<typename T> T boxkernel(int width)
    {
        T result;
        double rwidth = 1.0/width;
        for(int i = 0; i < width; i++)
        {
            result.push_back(rwidth);
        }
        return result;
    }

    // Return a convolution kernel with gaussian shape and area 1
    std::vector<double> gausskernel(int width)
    {
        return gausskernel< std::vector<double> > (width);
    }

    template<typename T> T gausskernel(int width)
    {
        T result;
        Sam::vector_traits<T>::do_reserve (result, width);

        double sigma = width/9.0;
        double rsigma = 1./sigma;
        double rsqrt2pi = 0.39894228040143267794;
        double a = rsqrt2pi*rsigma;
        double hwidth = width*0.5;
        for(int i = 0; i < width; i++)
        {
            result.push_back (a * exp((-0.5*(i-hwidth)*(i-hwidth))*rsigma*rsigma));
        }
        return result;
    }

    // Generates a moving window decomposition kernel
    std::vector<double> mwdkernel(int width, double epsilon)
    {
        return mwdkernel< std::vector<double> > (width, epsilon);
    }

    template<typename T> T mwdkernel(int width, double tau)
    {
        T result;
        result.push_back(1);
        double rtau = 1.0/tau;
        for(int i = 0; i < width; i++)
        {
            result.push_back(rtau);
        }
        result.push_back(-1);
        return result;
    }

    // Generates a moving window decomposition kernel without approximation of 1.0-exp(-1/tau)
    std::vector<double> mwdrealkernel(int width, double epsilon)
    {
        return mwdrealkernel< std::vector<double> > (width, epsilon);
    }

    template<typename T> T mwdrealkernel(int width, double tau)
    {
        T result;
        result.push_back(1);
        double val = 1.0-exp(-1.0/tau);
        for(int i = 0; i < width; i++)
        {
            result.push_back(val);
        }
        result.push_back(-1);
        return result;
    }

    // Generates a poissonian kernel
    std::vector<double> poissonkernel(double lambda) {
        return poissonkernel< std::vector<double> > (lambda);
    }

    template<typename T> T poissonkernel(double lambda)
    {
        T result;
        for(unsigned int k = 0; k < lambda*5; k++)
        {
            //      value = pow(lambda,k)*exp(-lambda);
            double value = gsl_ran_poisson_pdf(k,lambda);
            result.push_back(value);
        }
        return result;
    }

    // Returns an integer range from <start> to <stop> incremented by <increment>
    std::vector<int>    range(int start, int stop,int increment)
    {
        return range< std::vector<int> > (start, stop, increment);
    }

    template<typename T, typename V> T range(V start, V stop, V increment)
    {
        T result;
        for(V i = start; i < stop; i += increment)
        {
            result.push_back(i);
        }
        return result;
    }

    // Generates a sinc kernel
    std::vector<double> sinckernel(double width)
    {
        return sinckernel< std::vector<double> > (width);
    }

    template<typename T> T sinckernel(double width)
    {
        T result;
        double B = M_PI/width;
        for(int i = -width; i <= width; i++)
        {
            double value = 2*B*gsl_sf_sinc(2*B*i);
            result.push_back(value);
        }
        return result;
    }

    // Generates a negative ramp filter kernel with downward slope
    std::vector<double> rampdownkernel(int width)
    {
        return rampdownkernel< std::vector<double> > (width);
    }

    template<typename T> T rampdownkernel(int width)
    {
        T result;
        Sam::vector_traits<T>::do_reserve (result, width);
        for(int i = 0; i < width; i++)
        {
            result.push_back(1.0-(1.0*i/width));
        }
        return result;
    }

    // Returns a vector with zeroes
    std::vector<double> zeroes(unsigned int width)
    {
        return zeroes< std::vector<double> > (width);
    }

    template<typename T> T zeroes(unsigned int width)
    {
        T result;
        Sam::vector_traits<T>::do_fill (result, width, 0);
        return result;
    }

    // Window generators
    std::vector<double> blackmannuttalwindow(int width)
    {
        std::vector<double> result(width,0);
        static const double a0 = 0.3635819;
        static const double a1 = 0.4891775;
        static const double a2 = 0.1365995;
        static const double a3 = 0.0106411;
        double rwidthm1 = 1.0/(width-1);

        for(int i = 0; i < width; i++)
        {
            result[i] = a0 - a1*cos(2*M_PI*i*rwidthm1) + a2*cos(4*M_PI*i*rwidthm1) - a3*cos(6*M_PI*i*rwidthm1);
        }

        return result;
    }

    // Boolean operators
    std::vector<double> AND(const std::vector<double> & mask1, const std::vector<double> & mask2, int left, int right)
    {
        if(mask1.size() != mask2.size())
        {
            fprintf(stderr,"WARNING:SamDSP::AND: Mask sizes do not match (%d, %d). Returning mask1\n",(int)mask1.size(),(int)mask2.size());
            return mask1;
        }

        std::vector<double> result(mask1.size(),0);
        for(unsigned int i = 0; i < mask1.size(); i++)
        {
            if(mask1.at(i) > 0)
            {
                for(int j = -left; j <= right; j++)
                {
                    if((i+j) > 0 && (i+j) < mask1.size())
                    {
                        if(mask2.at(i+j) > 0)
                        {
                            result[i] = 1;
                            j = right+1;
                        }
                    }
                }
            }
        }
        return result;
    }

    std::vector<double>  OR(const std::vector<double> & mask1, const std::vector<double> & mask2, int left, int right);
    std::vector<double> NOT(const std::vector<double> & mask1, const std::vector<double> & mask2, int left, int right);
    std::vector<double> XOR(const std::vector<double> & mask1, const std::vector<double> & mask2, int left, int right);

    // Simulated Signals

    // Distribute randomly <num> events over std::vector with size <size>
    std::vector<double> randomevents(int size, int num, int seed)
    {
        // Generate vector with size <size> and <num> randomly distributed event markers
        std::vector<double> result(size,0);
        srand(seed);

        for (int i = 0;i < num; i++)
        {
            double pos =  static_cast<int>(((1.0*rand())/RAND_MAX)*size);
            result[pos]++;
        }
        return result;
    }

    // Return exponential signal prototype with decay constant tau
    std::vector<double> prototypePMT(int size, double tau)
    {
        // Make prototype signal
        std::vector<double> tmp = zeroes(size);
        for(int i = 0; i != static_cast<int>(tmp.size()); i++)
        {
            tmp[i] = exp(-static_cast<double>(i)/tau);
        }

        return tmp;
    }

    std::vector<double> prototypeTrace(int size, int nofSignals, double amplitude, const std::vector <double> & p)
    {
        // Make prototype trace
        std::vector<double> trace(size);
        for(int i = 0; i < nofSignals; i++)
        {
            int where = static_cast<int>(1.0*rand()/RAND_MAX)*size;
            trace[where] += amplitude;
        }
        trace = convolve(trace,p);
        return trace;
    }

    // Produce statistical distribution of <num> photons following an exponantial decay (scintillator)
    std::vector<double> statisticsScint(int num, double tau)
    {
        int size = static_cast<int>(tau*10);
        std::vector<double> tmp = zeroes(size);
        int pos;
        int i=0;
        double xvalue,yvalue,value;

        while(i <= num)
        {
            xvalue = ((1.0*rand())/RAND_MAX)*size;
            yvalue = exp(-xvalue/tau);
            value = ((1.0*rand())/RAND_MAX);
            if (value <= yvalue)
            {
                pos = static_cast<int>(xvalue);
                tmp[pos]++;
                i++;
            }
        }
        return tmp;
    }

    // Creates series noise signal
    std::vector<double> createSeriesNoise(unsigned int num, double size, int len)
    {
        std::vector<double> result(len,0);
        for(unsigned int i = 0; i < num; i++)
        {
            result[((1.0*rand())/RAND_MAX)*len] += ((1.0*rand())/RAND_MAX)*size;
            result[((1.0*rand())/RAND_MAX)*len] -= ((1.0*rand())/RAND_MAX)*size;
        }
        return result;
    }

    // Creates parallel noise signal
    std::vector<double> createParallelNoise(unsigned int num, double size, int len)
    {
        std::vector<double> result(len,0);
        for(unsigned int i = 0; i < num; i++)
        {
            result[((1.0*rand())/RAND_MAX)*len] += ((1.0*rand())/RAND_MAX)*size;
        }
        return result;
    }

    // DSP

    // Calculate signal2noise ratio in db
    double snr(const std::vector<double> & data, unsigned int from, unsigned int to, double mean, double max)
    {
        double snr = 0;
        unsigned int size = data.size();
        if(from > size || to > size || (to-from-1) <= 0)
        {
            fprintf(stderr,"ERROR in SamDSP::snr(): Index out of range\n");
            exit(1);
        }
        if(size < 1)
        {
            fprintf(stderr,"ERROR in SamDSP::snr(): Vector empty\n");
            exit(1);
        }

        for(unsigned int i = from; i < to; i++)
        {
            snr += (data[i]-mean)*(data[i]-mean);
        }
        snr /= (to-from-1);
        snr = 10.*log10(max*(max/snr));

        return snr;
    }

    // Returns a vector with trigger timestamps, triggered on local maximum
    template<typename T>
    T triggerLMT(const T & v, double threshold, int holdoff, int* nofTriggers=NULL)
    {
        unsigned int vsize = v.size();
        T result(vsize,0);
        unsigned int i = 3;
        unsigned int triggerCount = 0;
        while(i < vsize-2)
        {
            if((v[i] > threshold)
                && (v[i-1] <= v[i])
                && (v[i] > v[i+1])
                && (v[i-2] <= v[i])
                && (v[i] > v[i+2]))
            {
                result[i] = 1;
                i += holdoff;
                triggerCount++;
            }
            i++;
        }

        //cout << "TriggerLMT: Count: " << triggerCount << " in " << vsize << " samples." << endl
        //     << "Estimated frequency: " << (double)(triggerCount)/((double)(vsize)*2.)*1e9 << endl;

        if(nofTriggers != NULL) *nofTriggers = triggerCount;

        return result;
    }

    // Returns a vector with trigger timestamps, triggered on trailing edge
    std::vector<double> triggerTET(std::vector<double> v, double threshold, int holdoff)
    {
        std::vector<double> result(v.size(),0);
        unsigned int i = 2;
        while(i < v.size()-1)
        {
            if((v.at(i) > threshold) & (v.at(i-1) >= v.at(i)) & (v.at(i) > v.at(i+1)))
            {
                result[i] = 1;
                i += holdoff;
            }
            i++;
        }

        return result;
    }

    // Returns a vector with timestamps, triggered on leading edge
    std::vector<double> triggerLET(std::vector<double> v, double threshold, int holdoff, int* _nofTriggers=0)
    {
        int nofTriggers = 0;
        std::vector<double> result(v.size(),0);
        unsigned int i = 2;
        while(i < v.size()-1)
        {
            if((v.at(i) > threshold) & (v.at(i-1) < v.at(i)) & (v.at(i) <= v.at(i+1)))
            {
                result[i] = 1;
                i += holdoff;
            nofTriggers++;
            }
            i++;
        }

        if(_nofTriggers != 0) *_nofTriggers = nofTriggers;

        return result;
    }

    // Generate trigger when sum of differences reaches threshold, with holdoff
    std::vector<double> triggerTRI(std::vector<double> v, double threshold, unsigned int peaklength, unsigned int gatelength, int holdoff)
    {
        std::vector<double> result(v.size(),0);
        double sum1 = 0, sum2 = 0;
        int holdoffCnt = 0;
        bool overThr = false;

        // Init sums
        for(unsigned int i=0;i<peaklength && i<static_cast<unsigned int>(v.size());i++)
        {
            sum1 += v.at(i);
            sum2 += v.at(i+gatelength);
        }

        // Check initial value
        if(sum1 > (threshold+sum2))
        {
            overThr = true;
            result[0] = 1;
        }

        // Run through the trace
        for(unsigned int i=peaklength;(i+gatelength)<static_cast<unsigned int>(v.size());i++)
        {
            bool ltTransition = false;
            bool gtTransition = false;

            // Advance
            sum1 += v.at(i);
            sum2 += v.at(i+gatelength);
            sum1 -= v.at(i-peaklength);
            sum2 -= v.at(i+gatelength-peaklength);

            if(holdoffCnt > 0)
            {
                holdoffCnt--;
                continue;
            }

            if(sum1 >= (threshold+sum2))
            {
                if(!overThr) gtTransition = true;
                overThr = true;
            }
            else
            {
                if(overThr) ltTransition = true;
                overThr = false;
            }

            if(ltTransition)
            {
                continue;
            }
            if(gtTransition)
            {
                result[i-peaklength+1] = 1;
                holdoffCnt = holdoff;
            }
        }

        return result;
    }

    // Return the integrated vector
    std::vector<double> integrate(const std::vector<double> & v)
    {
        std::vector<double> result(v.size(),0);
        result[0] = v.at(0);
        for(unsigned int i = 1; i < v.size(); i++)
        {
            result[i] = v.at(i)+result.at(i-1);
        }
        return result;
    }

    // Reduces the amount of samples by step
    // First applies a truncated sinc filter to smooth the signal
    // Then returns a vector with only each <step> point
    std::vector<double> resample(const std::vector<double> & v, unsigned int step)
    {
        if(step == 1)
        {
            std::vector<double> result = v;
            return result;
        }
        if(2*step > v.size()-1)
        {
            printf("Warning:SamDSP::resample: Stepsize too large for signal vector\n");
            return v;
        }

        std::vector<double> result;
        std::vector<double> fs = sinckernel(step);
        std::vector<double> bn = blackmannuttalwindow(step*2+1);
                       fs = mul(fs,bn);

        // Only compute the convolution for the points that will be taken
        for(unsigned int i = 0; i < v.size() - fs.size(); i+=step)
        {
            double value = 0;
            for(unsigned int k = 0; k < fs.size(); k++)
            {
                value += v.at(i+k) * fs.at(k);
            }
            result.push_back(value);
        }
        return result;
    }

    // Restore the baseline based on averages between signals defined by mask
    // Each signal is expected to extend from each time in <mask> <left> samples
    // to the left and <right> samples to the right
    std::pair<int,double> restoreBaseline(std::vector<double> & v, const std::vector<double> & mask, int left, int right)
    {

        int cnt = 0;
        double baseline = 0;

        std::vector<double> selection = select(mask,mask)[TIME];

        if(selection.size() < 3)
        {
            fprintf(stderr,"WARNING: SamDSP::restoreBaseline: not enough timestamps!\n");
            return std::pair<int,double>(0,-1);
        }

        for(unsigned int i = 1; i < selection.size()-1; i++)
        {
            if(selection.at(i) > left && selection.at(i) < v.size()-right
               && (selection.at(i)-selection.at(i-1)) > (left+right))
            {
                for(unsigned k = selection.at(i-1)+right; k < selection.at(i)-left; k++)
                {
                    baseline += v[k];
                    cnt++;
                }
                if(cnt > 1000) break; // stop, at most 1000 samples
            }
        }
        baseline /= cnt;

        fast_addC(v,-baseline);

        return std::pair<int,double>(cnt,baseline);
    }

    // Unpile the amplitudes via matrix inversion
    std::vector<double> unpile(const std::vector<double> & timestamps, const std::vector<double> & amplitudes,
                                  const std::vector<int> & dimensions, const std::vector<double> & filtrate, double tzero)
    {
        if(timestamps.size() != amplitudes.size())
        {
            fprintf(stderr,"ERROR in SamDSP::unpile: More timestamps than amplitudes\n");
            fflush(stderr);
        }
        if(dimensions.size() != 4)
        {
            fprintf(stderr,"ERROR in SamDSP::unpile: Dimensions vector has wrong size\n");
            fflush(stderr);
        }
        std::vector<double> result;

        int preDim = dimensions[0];
        int postDim = dimensions[1];
        int preOverlap = dimensions[2];
        int postOverlap = dimensions[3];

        int n = timestamps.size();
        int nf = filtrate.size();
        std::vector<double> padAmps = pad(amplitudes,preDim,postDim,0);
        int dim = preDim + 1 + postDim;

        std::vector<std::vector<double> > iden = identity(dim);
        std::vector<std::vector<double> > m;

        // Unpile
        for(int k = 0; k < n; k++)
        {
            m = iden;
            for(int i = -preDim; i < postDim; i++)
            {
                int ki = k + i;
                for(int j = i - preOverlap; j < i + postOverlap; j++)
                {
                    int kj = k + j;
                    if((i != j) & within(ki,0,n-1) & within(kj,0,n-1))
                    {
                        int dt = timestamps[ki] - timestamps[kj];
                        int idx = floor(tzero) + dt;
                        if(within(idx,0,nf-1))
                        {
                            int mi = i + preDim;
                            int mj = j + preDim;
                            if(within(mi,0,dim-1) & within(mj,0,dim-1))
                            {
                                m[mi][mj] = filtrate[idx];
                            }
                        }
                    }
                }
            }

            // Inversion
            if(preDim > 1 || postDim > 1)
            {
                std::vector<std::vector<double> > mi = gslSvdInverse(m);
                std::vector<double>          miRow = mi[preDim];
                std::vector<double>          sliceAmps(dim);
                copy(padAmps.begin()+k, padAmps.begin()+k+preDim+postDim, sliceAmps.begin());
                double tmp = 0;
                for(unsigned int i = 0; i < miRow.size(); i++) // calculate scalar product
                {
                    tmp += miRow[i] * sliceAmps[i];
                }
                result.push_back(tmp);

                sliceAmps.clear();
                miRow.clear();
                mi.clear();
            }
            else
            {
                double tmp = 0;
                if(k == 0 || k == n-1)
                {
                    // Edge signals can not be corrected
                    tmp = amplitudes[k];
                }
                else
                {
                    // Simple analytical inversion for first neighbor approximation

                    /*
                     *   / 1  r  0 \ / a1 \   / b1 \
                     *   | p  1  s | | a2 | = | b2 |
                     *   \ 0  q  1 / \ a3 /   \ b3 /
                     */

                    double indexpr = (timestamps[k] - timestamps[k-1]);
                    double indexqs = (timestamps[k+1] - timestamps[k]);
                    double p,r,q,s;

                    // Check, if necessary values are in the lookup table
                    if(tzero + indexpr > 0 && tzero + indexpr < nf)
                        p = filtrate[tzero + indexpr]; // m[i-1][j]
                    else
                        p = 0;
                    if(tzero - indexpr > 0 && tzero - indexpr < nf)
                        r = filtrate[tzero - indexpr]; // m[i][j-1]
                    else
                        r = 0;
                    if(tzero + indexqs > 0 && tzero + indexqs < nf)
                        q = filtrate[tzero + indexqs]; // m[i][j+1]
                    else
                        q = 0;
                    if(tzero - indexqs > 0 && tzero - indexqs < nf)
                        s = filtrate[tzero - indexqs]; // m[i+1][j]
                    else
                        s = 0;
                    double eps = 0.0000000001;

                    double denominator = 1.0-p*r-q*s;

                    if(denominator < eps)
                    {
                        tmp = amplitudes[k];
                    }
                    else
                    {
                        // Usual matrix inversion
                        tmp = (amplitudes[k] - r*amplitudes[k-1] - q*amplitudes[k+1]) / denominator;
                        // Taking into account the last unpiled amplitude (more iterative approach)
                        //tmp = (amplitudes.at(k) - r*result.back() - q*amplitudes.at(k+1)) / denominator;
                    }
                }

                result.push_back(tmp);
            }

            m.clear();
        }

        padAmps.clear();

        return result;
    }

    // Returns QDC values for gates of times [t-left;t+right], terminated by max
    std::vector<double> QDC(const std::vector<double> & data, const std::vector<double> & t, unsigned int left, unsigned int right, unsigned int max)
    {
        unsigned int tsize = t.size();
        std::vector<double> result(tsize,0);
        double value = 0;

        for(unsigned int i = left+1; i < (tsize-right+1) && i < max; i++)
        {
            if (t[i] == 1)
            {
                value = 0;
                for(unsigned int z = i-left; z < (i+right); z++)
                {
                      value += data[z];
                }
                result[i] = value;
            }
        }

        return result;
    }

    // Returns QDC values for gates of times in listmode [t-left;t+right]
    std::vector<double> listQDC(const std::vector<double> & data, const std::vector<double> & t, unsigned int left, unsigned int right)
    {
        std::vector<double> result;
        double value = 0;

        for(unsigned int i = left+1; i < (t.size()-right+1); i++)
        {
            if (t[i] == 1)
            {
                value = 0;
                for(unsigned int z = i-left; z < (i+right); z++)
                {
                      value += data[z];
                }
                result.push_back(value);
            }
        }

        return result;
    }

    // Return TDC values for two different timing masks in listmode
    std::vector<double> listTDC(const std::vector<double> & mask1, const std::vector<double> & mask2, unsigned int left, unsigned int right)
    {
        if(mask1.size() != mask2.size())
        {
            fprintf(stderr,"WARNING:SamDSP::AND: Mask sizes do not match (%d, %d). Returning mask1\n",(int)mask1.size(),(int)mask2.size());
            return mask1;
        }

        std::vector<double> result;
        for(unsigned int i = 0; i < mask1.size(); i++)
        {
            if(mask1.at(i) > 0)
            {
                for(int j = -left; j <= static_cast<int>(right); j++)
                {
                    if((i+j) > 0 && (i+j) < mask1.size())
                    {
                        if(mask2.at(i+j) > 0)
                        {
                            result.push_back(j);
                        }
                    }
                }
            }
        }
        return result;
    }

    // Calculate matrix projection over rows (e.g. add all signals up)
    std::vector<double> projectRows(std::vector<std::vector<double> > &data)
    {
        return projection(data, true);
    }

    // Calculate matrix projection over columns (e.g. integrate all signals)
    std::vector<double> projectColumns(std::vector<std::vector<double> > &data)
    {
        return projection(data, false);
    }

    // Calculate a matrix projection, either on columns or rows
    // (return the projection vector)
    std::vector<double> projection(std::vector<std::vector<double> > &data, bool sumOverRows)
    {
        std::vector<double> result;

        if(data.size() < 1)
        {
            fprintf(stderr,"ERROR:SamDSP::projection No vectors in dataset!\n");
            return result;
        }
        if(data.at(0).size() < 1)
        {
            fprintf(stderr,"ERROR:SamDSP::projection No points in dataset!\n");
            return result;
        }

        if(sumOverRows == true)
        {
            result.resize(data.at(0).size(),0);
            for(unsigned int i = 0; i < data.size(); i++)
            {
                result = add(result,data.at(i));
            }
        }
        else
        {
            result.resize(data.size(),0);
            for(unsigned int i = 0; i < data.at(0).size(); i++)
            {
                for(unsigned int j = 0; j < data.size(); j++)
                {
                    result[j] += data.at(j).at(i);
                }
            }
        }

        return result;
    }

    // Get constant fraction timing based on tz starting points
    //  tz points should result from local maximum search on timing filtered signals
    //  vector v should be maximum-aligned with timing filtered signal
    //
    // This will mark precise timings in a trace and calculate the phase to achieve sub-integer precision
    // Fraction is the Constant fraction threshold
    template<typename T>
    typename Sam::vector_traits<T>::vecvec_type triggerCFD(const T & v, const T & vtz, double fraction, int holdoff)
    {
        typename Sam::vector_traits<T>::vecvec_type result(2,T (v.size(),0));

        if(v.size() != vtz.size())
        {
            fprintf(stderr,"WARNING:SamDSP::triggerCFD: Vector sizes do not match (%d, %d)\n",(int)v.size(),(int)vtz.size());
            return result;
        }

        unsigned int i = 2, tz = 0;
        double phase = 0;
        while(i < static_cast<unsigned> (vtz.size())-1)
        {
            if(vtz.at(i) == 1)  // This is the amplitude of each triggered signal
            {
                tz = i;

                // CFD routine (go backwards in time until below constant fraction)
                while(v.at(tz) > v.at(i)*fraction && tz > 0)
                {
                    tz--;
                }

                phase = (v.at(i)*fraction-v.at(tz)) / (v.at(tz+1)-v.at(tz));

                result[TIME][tz] = 1;
                result[PHASE][tz] = phase;

                //fprintf(stdout,"%d> Phase: %f, l: %f, h: %f\n",tz,phase,v.at(tz),v.at(tz+1));
                i += holdoff;
            }
            i++;
        }

        return result;
    }

    // Same as above, but without a holdoff setting
    template<typename T>
    typename Sam::vector_traits<T>::vecvec_type triggerCFD(const T & v, const T & vtz, double fraction)
    {
        typename Sam::vector_traits<T>::vecvec_type result(2,T (v.size(),0));

        if(v.size() != vtz.size())
        {
            fprintf(stderr,"WARNING:SamDSP::triggerCFD: Vector sizes do not match (%d, %d)\n",(int)v.size(),(int)vtz.size());
            return result;
        }

        unsigned int i = 2, tz = 0;
        double phase = 0;
        while(i < vtz.size()-1)
        {
            if(vtz.at(i) == 1)  // This is the amplitude of each triggered signal
            {
                tz = i;

                // CFD routine (go backwards in time until below constant fraction)
                while(v.at(tz) > v.at(i)*fraction && tz > 0)
                {
                    tz--;
                }

                phase = (v.at(i)*fraction-v.at(tz)) / (v.at(tz+1)-v.at(tz));

                result[TIME][tz] = 1;
                result[PHASE][tz] = phase;

                //fprintf(stdout,"%d> Phase: %f, l: %f, h: %f\n",tz,phase,v.at(tz),v.at(tz+1));
            }
            i++;
        }

        return result;
    }

    // Same as above, but also output rise time
    std::vector<std::vector<double> > triggerCFDRT(const std::vector<double> & v, const std::vector<double> & vtz, double fraction)
    {
        std::vector<std::vector<double> > result(3,std::vector<double>(v.size(),0));

        if(v.size() != vtz.size())
        {
            fprintf(stderr,"WARNING:SamDSP::triggerCFD: Vector sizes do not match (%d, %d)\n",(int)v.size(),(int)vtz.size());
            return result;
        }

        unsigned int i = 2, tz = 0;
        double phase = 0;
        while(i < vtz.size()-1)
        {
            if(vtz.at(i) == 1)  // This is the amplitude of each triggered signal
            {
                tz = i;

                // CFD routine (go backwards in time until below constant fraction)
                while(v.at(tz) > v.at(i)*fraction && tz > 0)
                {
                    tz--;
                }

                phase = (v.at(i)*fraction-v.at(tz)) / (v.at(tz+1)-v.at(tz));

                result[TIME][tz] = 1;
                result[PHASE][tz] = phase;
                result[2][tz] = i-tz;

                //fprintf(stdout,"%d> Phase: %f, l: %f, h: %f\n",tz,phase,v.at(tz),v.at(tz+1));
            }
            i++;
        }

        return result;
    }

    // Analog CFD: Get constant fraction timing based on zero crossing
    //
    // This will mark precise timings in a trace and calculate the phase to achieve sub-integer precision
    // Fraction is the Constant fraction threshold
    template <typename T>
    typename Sam::vector_traits<T>::vecvec_type triggerACFD(const T & v, double thr, double fraction, int delay, int holdoff)
    {
        typename Sam::vector_traits<T>::vecvec_type result(3, T ());
        Sam::vector_traits<T>::do_fill (result [TIME], v.size (), 0);
        Sam::vector_traits<T>::do_fill (result [AMP], v.size (), 0);
        Sam::vector_traits<T>::do_fill (result [2], v.size (), 0);

        if (delay >= v.size ()) {
            fprintf (stderr, "WARNING: SamDSP::triggerACFD: Delay longer than signal!");
            return result;
        }

        for (int i = 1; i < static_cast<int> (v.size()) - delay; i++)
        {
            double z1 = v [i-1] - fraction * v [i+delay-1];
            double z2 = v [i] - fraction * v [i+delay];
            result [2][i-1] = z1; // cfd difference signal for testing
            // zero-crossing found and peak height sufficient
            if (sign (z1) * sign (z2) <= 0) {
                if (z1 == 0 && z2 == 0) // zero-crossing is detected when the signal becomes non-zero
                    continue;

                double phase = -z1 / (z2 - z1);
                int nearest = rint (i-1 + phase);

                if (fabs (v[nearest] / fraction) < thr)
                    continue;

                // find a value above the threshold (if it is not there the trigger was spurious)
                bool found = false;
                for (int j = i; j < i+holdoff && j < static_cast<int> (v.size()); ++j)
                    if (fabs (v [j]) >= thr) {
                        found = true;
                        break;
                    }

                if (!found)
                    continue;

                result [TIME][nearest] = 1;
                result [AMP][nearest] = i-1 + phase - nearest; // adjust phase

                i += holdoff - 1;
            }
        }

        return result;
    }

    // Calculate signal phase, when cfd timing of signal (v) is known to integer precision (mask)
    std::vector<std::vector<double> > phasesCFD(std::vector<double> v, std::vector<double> mask, double threshold)
    {
        std::vector<std::vector<double> > result(2);
        unsigned int i = 0;
        double phase;
        while(i < v.size())
        {
            if(mask.at(i) == 1)
            {
                phase = (threshold-v.at(i)) / (v.at(i+1)-v.at(i));
                result[TIME].push_back(i);
                result[AMP].push_back(phase);

                fprintf(stdout,"%d> Phase: %f\n",i,phase);
            }
            i++;
        }
        return result;
    }

    // FCM clustering algorithm takes the data, the number of classes to separate into and possibly a starting correlation matrix as arguments
    // Returns a vector containing the class prototypes
    std::vector<std::vector<double> > fcm(std::vector<std::vector<double> > data, double m, unsigned int nofClasses, std::vector<std::vector<double> > &u)
    {
        int nofIterations = 100;
        double normepsilon = 1e-16;
        double epsilon = 0.00001;
        int n = data.size();
        int np = 0;
        double alpha = m;

        if(n < 1)
        {
            fprintf(stderr,"ERROR:SamDSP::fcm No data present!\n");
            return u;
        }
        else
        {
            np = data.at(0).size();

            if(np < 1)
            {
                fprintf(stderr,"ERROR:SamDSP::fcm No points in data!\n");
                return u;
            }
        }

        printf("FCM with %d signals with %d points each.\n",n,np);

        // generate random matrix
        u.resize(n);
        for(int i = 0; i < n; i++)
        {
            u[i].resize(nofClasses);
            for(unsigned int j = 0; j < nofClasses; j++)
            {
                u[i][j] = ((1.0*rand())/RAND_MAX);
            }
            // Normalize row
            double norm = 0;
            for(unsigned int j = 0; j < nofClasses; j++)
            {
                norm += pow(u.at(i).at(j),2);
            }
            norm = sqrt(norm);
            u[i] = scale(u.at(i),1.0/norm);
        }

        // Centroid vectors
        std::vector<std::vector<double> > c(nofClasses,std::vector<double>(np,0));

        // Distance matrix
        std::vector<std::vector<double> > d(nofClasses,std::vector<double>(n,0));

        // FCM loop
        for(int it = 0; it < nofIterations; it++)
        {
            // Update centers (weighed average)
            for(unsigned int i = 0; i < nofClasses; i++)
            {
                std::vector<double> sum(np,0);
                double dSum = 0;

                for(int j = 0; j < n; j++)
                {
                    double uji;
                    uji = pow(u[j][i],alpha);
                    sum = add(sum,scale(data.at(j), uji));
                    dSum += uji;
                }
                c[i] = scale(sum, 1/dSum);
            }
            // Calculate distances
            for(unsigned int i = 0; i < nofClasses; i++)
            {
                std::vector<double> v(np,0);
                std::vector<double> w(np,0);

                for(int j = 0; j < n; j++)
                {
                    v = sub(data.at(j),c.at(i));
                    // Vectornorm
                    double norm = 0;
                    for(int k = 0; k < np; k++)
                    {
                        norm += pow(v.at(k),2);
                    }
                    norm = sqrt(norm);
                    if(norm < normepsilon) norm = normepsilon;
                    d[i][j] = norm;
                }
            }
            // Update correlation matrix u
            std::vector<std::vector<double> > unew(n,std::vector<double>(nofClasses,0));
            for(int i = 0; i < n; i++)
            {
                for(unsigned int j = 0; j < nofClasses; j++)
                {
                    double uij = 0;
                    for(unsigned int k = 0; k < nofClasses; k++)
                    {
                        uij += pow(d[j][i]/d[k][i] , 2.0/(alpha-1));
                    }
                    unew[i][j] = 1.0/uij;
                }
    //            // Renormalize row
    //            double norm = 0;
    //            for(unsigned int j = 0; j < nofClasses; j++)
    //            {
    //                norm += pow(u.at(i).at(j),2);
    //            }
    //            norm = sqrt(norm);
    //            u[i] = scale(u.at(i),1.0/norm);
            }
            // Matrixnorm (difference between old and new correlation matrix)
            double du = 0;
            for(int i = 0; i < n; i++)
            {
                for(unsigned int j = 0; j < nofClasses; j++)
                {
                    double tmp = u[i][j] - unew[i][j];
                    du += tmp*tmp;
                }
            }
            printf("FCM Iteration: %d, du: %f\r",it,du);fflush(stdout);
            if(du < epsilon)
            {
                it = nofIterations;
            }
            u = unew;
        }
        printf("\n");

        return c;
    }

    // Returns QDC values for gates of times [t-left;t+right]
    std::vector<double> QDC(const std::vector<double> & data, const std::vector<double> & t, unsigned int left, unsigned int right)
    {
        unsigned int tsize = t.size();
        std::vector<double> result(tsize,0);
        double value = 0;

        for(unsigned int i = left+1; i < (tsize-right+1); i++)
        {
            if (t[i] == 1)
            {
                value = 0;
                for(unsigned int z = i-left; z < (i+right); z++)
                {
                      value += data[z];
                }
                result[i] = value;
            }
        }

        return result;
    }


    // Compact the vector by removing space in between (signals)
    // From each peak in mask extract all samples [peak-left:peak+right] from the sigal
    // Returns a matrix containing the compacted vector and the compacted mask
    std::vector<std::vector<double> > compact(const std::vector<double> & v, const std::vector<double> & mask, int left, int right)
    {
        std::vector<double> compact;
        std::vector<double> compact_mask;

        for(int i = 0; i < static_cast<int>(mask.size()); i++)
        {
            if(mask.at(i) == 1)
            {
                if((i-left) > 0 && (i+right) < static_cast<int>(v.size()))
                {
                    for(int j = -left; j < right; j++)
                    {
                        compact.push_back(v.at(i+j));
                        if(j == 0) compact_mask.push_back(1);
                        else compact_mask.push_back(0);
                    }
                }
            }
        }
        std::vector<std::vector<double> > result;
        result.push_back(compact);
        result.push_back(compact_mask);

        return result;
    }

    // Produce histogram of values in data, histograms starts with value "min",
    // ends with value "max" and has "bins" bins
    std::vector<std::vector<double> > histVector(const std::vector<double> & data, double min, double max, unsigned int bins)
    {

        std::vector<std::vector<double> > hist;

        std::vector<double> xvalues;
        std::vector<double> yvalues;

        double step      = (max-min)/bins;
        double x         = min;
        double value     = 0;
        unsigned int bin = 0;

        for(unsigned int i = 0; i < bins; i++)
        {
            xvalues.push_back(x);
    //        cout << "Bin " << i << " -> " << x << endl;
            yvalues.push_back(0);
            x+=step;
        }

        unsigned int n = data.size();

        for(unsigned int i = 0; i < n; i++)
        {
            value = data[i];
            bin = rint((value-min)/step);
            if(bin > 0 && bin < bins)
            {
    //            cout << "Value " << value << " -> " << bin << endl;
                yvalues[bin]++;
            }
        }

        hist.push_back(xvalues);
        hist.push_back(yvalues);

        xvalues.clear();
        yvalues.clear();

        return hist;

    }

    // Adaptive leading edge trigger
    std::vector<std::vector<double> > triggerALED(const std::vector<double> & v, const std::vector<double> & model, double threshold, int risetime)
    {
        std::vector<std::vector<double> > result;
        (void) model; // unused variables
        (void) threshold;

        enum State{baseline,one,two,three,four,unknown};
        //State state = unknown;

        double bl=0, dbl=1000;
        //double a1=0, da1=1000, t1=0, dt1=1000;
        //double a2=0, da2=1000, t2=0, dt2=1000;
        //double a3=0, da3=1000, t3=0, dt3=1000;
        //double a4=0, da4=1000, t4=0, dt4=1000;

        double btau = 2.0/(2*risetime+1);

        //double nsum = 0;
        //double psum = 0;
        double width = ceil(risetime*0.5);

        if(v.size() < 2*width)
        {
            fprintf(stderr,"ERROR: SamDSP::triggerALED: Vector too small. Needs at least %d elements.\n",risetime);
        }

        std::vector<double> vbl;
        std::vector<double> vdbl;

        for(unsigned int i = 0; i < v.size(); i++)
        {
            // Predict baseline
            //double pbl = bl;
            // New baseline value
            double nbl = v[i];
            // New baseline estimate
            bl = btau * nbl + (1-btau) * bl;
            // Predict baseline error
            double pdbl = dbl;
            // New baseline error estimate
            dbl = btau * (bl-sqrt(pdbl))*(bl-sqrt(pdbl)) + (1-btau) * dbl;

            vbl.push_back(bl);
            vdbl.push_back(dbl);
        }

        result.push_back(vbl);
        result.push_back(vdbl);

        return result;
    }

    // Kalman filters
    template <typename T>
    int kalmanBaseline(const T & signal, T & x, double r, double ri, double q, double x0)
    {
        std::vector<double> p;
        std::vector<double> in;
        std::vector<double> bl_mask;

        // Initialization

        const T & v (signal);
        std::vector<double> in_x;
        std::vector<double> in_p;
        std::vector<double> rise_x, rise_p;
        std::vector<double> signal_mask;

        int lenOThrCnt = 3;

        x.push_back(x0);
        p.push_back(10.);
        in.push_back(0.);
        in_x.push_back(0.);
        in_p.push_back(10.);
        rise_x.push_back(0.);
        rise_p.push_back(1.);
        bl_mask.push_back(0.);
        signal_mask.push_back(0.);

        double in_mwa=0, in_mwv=1;

        // Kalman loop
        for(unsigned int i = 0; i < static_cast<unsigned> (v.size()); i++)
        {
            // #################### X ################
            // Predict x
            double px = x.back();
            double pp = p.back() + q;

            // Correct x
            double K  = pp / (pp + r);
            double I  = v[i] - px;
            double cx = px + K * I;
            double cp = (1 - K) * pp;

            // #################### I ###############
            // Predict I
            double pi  = in_x.back();
            double ppi = in_p.back();

            // Correct I
            double Ki = ppi / (ppi + ri);
            double Ii = I - pi;
            double ci = pi + Ki * Ii;
            double cpi = (1- Ki) * ppi;

            // #################### Rise ###############
            // Predict rise
    //        static double a = -1;
    //        double pr = rise_x.back() + a;
    //        double ppr = rise_p.back();

            // #################### Statistics ###############
            // MWA of innovation
            in_mwa = .2 * I + (0.8) * in_mwa;
            // MWV of innovation
            in_mwv = (.01 * (I - in_mwa) *
                                  (I - in_mwa)
                                + (0.99) * in_mwv);

            in_x.push_back(ci);
            in.push_back(I);

            // Calculate threshold
            double thr = 0;
            static int oThrCnt = 0;
            static int signalCnt = 0;
            if(sqrt(in_mwv) < sqrt(in_p.back()))
            {
                in_p.push_back(cpi);
                thr = sqrt(in_p.back())*1;
            }
            else
            {
                in_p.push_back(in_mwv);
                thr = sqrt(in_mwv)*1;
            }

            // Decide if baseline or signal
            if(in.back() > thr) oThrCnt++;
            else if(oThrCnt > 0) oThrCnt--;

            if(oThrCnt > lenOThrCnt)
            {
                // Signal: Push predicted values
                x.push_back(px);
                p.push_back(pp);
                bl_mask.push_back(0);
                oThrCnt--;
                signalCnt++;
            }
            else
            {
                // Baseline: Push corrected values
                x.push_back(cx);
                p.push_back(cp);
                bl_mask.push_back(1);
                signalCnt = 0;
            }

    //        // Decide, if signal is present
    //        if(signalCnt > lenSignalCnt && i > 1000)
    //        {
    //            //signal_mask.push_back(1);
    //            //double as = sum(in,i-lenInt,i);
    //            //double factor = as/am;
    //            //cout << am << "  " << factor << endl;
    //            //std::vector<double> modelScaled = scale(model,factor);
    //            //v = sub(v,modelScaled,i-lenInt-10);
    //            //in = sub(in,modelScaled,i-lenInt-10);
    //            //signalCnt = -10;
    //            //oThrCnt = -10;
    //        }
    //        else
    //        {
    //            signal_mask.push_back(0);
    //        }
        }

    //    vectorToFile(v,"/tmp/v");
    //    vectorToFile(in_x,"/tmp/inavg");
    //    vectorToFile(in_p,"/tmp/invar");
    //    vectorToFile(signal_mask,"/tmp/signal_mask");

        return 0;
    }

    // Convolution
    // Simple convolution algorithm
    std::vector<double> convolve(const std::vector<double> & v, const std::vector<double> & kernel)
    {
        std::vector<double> result;
        int ksize = kernel.size();
        int vsize = v.size();

        for(int k = 0; k < vsize+ksize-1; k++)
        {
            double yk = 0;
            for(int n = 0; n < ksize; n++)
            {
                if((k - n) >= 0 && (k - n) < vsize)
                {
                    double hn = kernel[n];
                    double xkn = 0;
                    xkn = v[k - n];
                    yk += xkn * hn;
                }
            }
            result.push_back(yk);
        }
        return result;
    }

    // Fast algorithms, not necessarily generic (feel free to improve)
    template<typename T>
    int fast_boxfilter(T & v, unsigned int width)
    {
        unsigned int n = v.size();
        if(width > 1 && n > width)
        {
            double sum = 0;
            double a = 1.0/width;
            for(unsigned int i = 1; i < width+1; i++)
            {
                sum += v[i];
            }
            v[0] = sum * a;
            for(unsigned int i = width+1; i < n; i++)
            {
                sum += v[i] - v[i-width];
                v[i-width] = sum * a;
            }
        }
        else
        {
            return 1;
        }
        return 0;
    }

    template <typename T>
    int fast_biboxfilter(T & v, unsigned int width)
    {
        unsigned int n = v.size();
        if(n > 2*width)
        {
            fast_differentiator(v,width,width);
        }
        else
        {
            return 1;
        }
        return 0;
    }

    template <typename T>
    int fast_differentiator(T & v, unsigned int width, unsigned int delay)
    {
        unsigned int n = v.size();
        if(n > width + delay)
        {
            if(width > 1)
            {
                double sum = 0;
                double a = 1.0;
                //double a = 1.0/width; //FIXME (this would be right, but must be propagated
                for(unsigned int i = 1; i < width+1; i++)
                {
                    sum += (v[delay+i] - v[i]) * a;
                }
                v[0] = sum;
                for(unsigned int i = width+1; i < (n-delay); i++)
                {
                    sum += (v[i+delay] - v[i+delay-width] - v[i] + v[i-width]) * a;
                    v[i-width] = sum;
                }
            }
            else
            {
                v[0] = 0;
                for(unsigned int i = 1; i < (n-delay); i++)
                {
                    v[i-1] = v[i+delay] - v[i];
                }
            }
        }
        else
        {
            return 1;
        }
        return 0;
    }

    template <typename T>
    int fast_add(T & a, const T & b)
    {
        if(a.size() != b.size())
        {
            fprintf(stderr,"ERROR in SamDSP::fast_add: Size must be equal (a = %d, b = %d\n",(int)a.size(),(int)b.size());
            fflush(stderr);
            return 1;
        }

        for(unsigned int i = 0; i < static_cast<unsigned> (a.size()); i++)
        {
                a[i] = a[i] + b[i];
        }

        return 0;
    }


    template<typename T>
    int fast_addC(T & a, double b)
    {
        for(int i = 0; i < static_cast<int> (a.size()); i++)
        {
                a[i] = a[i] + b;
        }

        return 0;
    }

    template<typename T>
    int fast_scale(T & a, double factor)
    {
        if(std::isnan(factor))
        {
            fprintf(stderr,"ERROR in SamDSP::scale: Factor is not a number\n");
            return 1;
        }

        for(int i = 0; i < static_cast<int> (a.size()); i++)
        {
                if(std::isnan(a[i] * factor)) a[i] = 0;
                else a[i] = a[i] * factor;
        }

        return 0;
    }

    template<typename T>
    int fast_pad(T & v, int left, int right, double value)
    {
        v.insert(v.begin(), left, value);
        v.insert(v.end(), right, value);
        return 0;
    }

    template<typename T>
    int fast_shift(T & v, int distance)
    {
        size_t n = v.size();
        if((unsigned int)(abs(distance)) > n)
        {
            fprintf(stderr,"ERROR in SamDSP::shift: Shift too far\n");
            return 1;
        }
        if(distance > 0)
        {
            std::rotate(v.begin(),v.begin()+distance,v.end());
            std::fill(v.begin(),v.begin()+distance,v[distance]);
    //        fast_pad(v,distance,0,v[0]);
    //        v.resize(n);
        }
        else if(distance < 0)
        {
            std::rotate(Sam::rev_iter (v.end ()), Sam::rev_iter (v.end ())-distance,Sam::rev_iter (v.begin ()));
            std::fill(v.end()+distance,v.end(),v[n+distance]);
    //
    //        fast_pad(v,0,-distance,v[n-1]);
    //
    //        while(v.size() > n)
    //        {
    //            v.erase(v.begin());
    //        }
        }

        return 0;
    }

    // Output functions

    int vectorPrint(const std::vector<double> & v)
    {
        // Print the vector
        std::vector<double>::const_iterator vit(v.begin());
        for(int i = 0; vit != v.end(); vit++, i++)
        {
            fprintf(stdout,"<%d> %f\n",i,*vit);
        }
        return 0;
    }

    template<typename T>
    int vectorToFile(const T & v, std::string fileName)
    {
        if(v.size() > 0)
        {
            // Print the vector
            std::ofstream file;
            file.open(fileName.c_str());
            for(unsigned int i = 0; i < static_cast<unsigned> (v.size()); i++)
            {
                file << v[i] << std::endl;
            }
            file.close();
            return 0;
        }
        else
        {
            return -1;
        }
        return -1;
    }

    template <typename T>
    int vectorFromFile(T & v, std::string fileName)
    {
        std::ifstream file;
        file.open(fileName.c_str());
        v.clear();
        while(file.good())
        {
            typename Sam::vector_traits<T>::value_type val;
            file >> val;
            v.push_back(val);
        }
        file.close();
        return v.size();
    }

    template<typename T>
    int vectorToFile(const T & v, std::string fileName, unsigned int from, unsigned int to)
    {
        if(to >= static_cast<unsigned> (v.size()))
        {
            fprintf(stderr,"ERROR in SamDSP::vectorToFile: out of range\n");
            return 1;
        }

        // Print the vector with boundary conditions
        std::ofstream file;
        file.open(fileName.c_str());
        for(unsigned int i = from; i < to; i++)
        {
            file << v[i] << std::endl;
        }
        file.close();
        return 0;
    }

    int matrixPrint(const std::vector<std::vector<double> > & m, bool transpose=false)
    {
        // Print the matrix
        fprintf(stdout,"\n");
        for(unsigned int row = 0; row < (transpose ? m[0].size() : m.size()); row++)
        {
            for(unsigned int col = 0; col < (transpose ? m.size() : m[row].size()); col++)
            {
                fprintf(stdout,"%f ",m[row][col]);
            }
            fprintf(stdout,"\n");
        }
        return 0;
    }

    int matrixToFile(const std::vector<std::vector<double> > & m, std::string fileName, bool transpose=false)
    {
        // Check for data
        if(m.empty()) return 1;

        // Print the matrix
        std::ofstream file;
        file.open(fileName.c_str());

        for(unsigned int row = 0; row < (transpose ? m[0].size() : m.size()); row++)
        {
            for(unsigned int col = 0; col < (transpose ? m.size() : m[row].size()); col++)
            {
                if(transpose)
                    file << m[col][row] << " ";
                else
                    file << m[row][col] << " ";
            }
            file << std::endl;
        }
        file.close();
        return 0;
    }
};

#endif // SAMDSP_H
