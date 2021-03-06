/*   Copyright 2013 Juan Rada-Vilela

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

/*
 * Variable.h
 *
 *  Created on: 2/12/2012
 *      Author: jcrada
 */

#ifndef FL_VARIABLE_H
#define FL_VARIABLE_H

#include "fl/fuzzylite.h"


#include "fl/defuzzifier/Centroid.h"
#include "fl/operator/Operator.h"

#include <string>
#include <vector>
#include <limits>

namespace fl {

    class Term;
    
    class FL_EXPORT Variable {
    protected:
        std::string _name;
        std::vector<Term*> _terms;
        scalar _minimum, _maximum;

        struct SortByCoG {
            scalar minimum, maximum;
            Centroid cog;

            bool operator() (const Term* a, const Term * b) {
                return fl::Op::isLt(
                        cog.defuzzify(a, minimum, maximum),
                        cog.defuzzify(b, minimum, maximum));
            }
        };

    public:
        Variable(const std::string& name = "",
                scalar minimum = -std::numeric_limits<scalar>::infinity(),
                scalar maximum = std::numeric_limits<scalar>::infinity());
        Variable(const Variable& copy);

        virtual ~Variable();

        virtual void setName(const std::string& name);
        virtual std::string getName() const;
        
        virtual void setRange(scalar minimum, scalar maximum);

        virtual void setMinimum(scalar minimum);
        virtual scalar getMinimum() const;

        virtual void setMaximum(scalar maximum);
        virtual scalar getMaximum() const;

        virtual std::string fuzzify(scalar x) const;
        virtual Term* highestMembership(scalar x, scalar* yhighest = NULL) const;

        virtual std::string toString() const;

        /**
         * Operations for iterable datatype _terms
         */
        virtual void sort();
        virtual void addTerm(Term* term);
        virtual void insertTerm(Term* term, int index);
        virtual Term* getTerm(int index) const;
        virtual Term* getTerm(const std::string& name) const;
        virtual bool hasTerm(const std::string& name) const;
        virtual Term* removeTerm(int index);
        virtual int numberOfTerms() const;
        virtual const std::vector<Term*>& terms() const;

    };

}

#endif /* FL_VARIABLE_H */
