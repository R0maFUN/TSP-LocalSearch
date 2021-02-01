#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>

class City
{
private:
    std::size_t id;
    int x;
    int y;
    double penalty;
public:
    City(std::size_t id, int x, int y) : id(id), x(x), y(y), penalty(0) {}
    float getWay(const City& dest) {
        return (std::sqrt(pow((this->x - dest.x), 2) + pow((this->y - dest.y), 2)));
    }
    std::size_t getId() const {
        return this->id;
    }

    int getX() const {
        return this->x;
    }

    int getY() const {
        return this->y;
    }

    void addPenalty(double amount) {
        this->penalty += amount;
    }

    void removePenalty() {
        this->penalty = 0;
    }

    double getPenalty() const {
        return this->penalty;
    }

};

class Way {
private:
    std::shared_ptr<City> cityA;
    std::shared_ptr<City> cityB;
    long float length;
    double penalty;
public:
    Way(std::shared_ptr<City> A, std::shared_ptr<City> B) : cityA(A), cityB(B) {
        calculateLength();
    }

    long float calculateLength() {
        this->length = std::sqrt(pow((this->cityA->getX() - this->cityB->getX()), 2) + pow((this->cityA->getY() - this->cityB->getY()), 2));
        return this->length;
    }
};

class Solution {
protected:
    std::vector<std::shared_ptr<City> > cities;
    long float length;
    std::size_t citiesAmount;
public:
    Solution(const std::vector<std::vector<int> >& coords) {
        for (auto row : coords) {
            cities.push_back(std::shared_ptr<City>(new City(row[0], row[1], row[2])));
        }
        calculateLength();
        citiesAmount = cities.size();
    }

    Solution(const std::string filename) {
        std::ifstream inFile;
        inFile.open(filename);
        int count;
        inFile >> count;
        citiesAmount = count;
        for (std::size_t i{ 0 }; i < count; ++i) {
            std::vector<int> row(3);
            inFile >> row[0] >> row[1] >> row[2];
            cities.push_back(std::shared_ptr<City>(new City(row[0], row[1], row[2])));
        }
        inFile.close();
        calculateLength();
    }

    Solution(const Solution& val) {
        this->cities = val.cities;
        calculateLength();
        this->citiesAmount = val.citiesAmount;
    }

    long float calculateLength() {
        this->length = 0;
        for (std::size_t i{ 0 }; i < cities.size() - 1; ++i) {
            this->length += cities[i]->getWay(*(cities[i + 1]));
        }
        this->length += cities[cities.size() - 1]->getWay(*cities[0]);
        return this->length;
    }

    void replaceRange(std::size_t id1, std::size_t id2) {
        auto firstItr = std::find_if(cities.begin(), cities.end(), [id1](const auto& val) { return val->getId() == id1; });
        auto secondItr = std::find_if(cities.begin(), cities.end(), [id2](const auto& val) { return val->getId() == id2; });
        std::reverse(firstItr < secondItr ? firstItr : secondItr, firstItr < secondItr ? ++secondItr : ++firstItr);
    }

    void replace(std::size_t id1, std::size_t id2) {
        std::iter_swap(std::find_if(cities.begin(), cities.end(), [id1](const auto& val) { return val->getId() == id1; })
            , std::find_if(cities.begin(), cities.end(), [id2](const auto& val) { return val->getId() == id2; }));
    }

    void print() {
        for (const auto& row : cities) {
            std::cout << row->getId() << " ";
        }
        std::cout << "\n";
    }

    void print(std::string filename) {
        std::ofstream outFile;
        outFile.open(filename);
        for (const auto& row : cities) {
            outFile << row->getId() << " ";
        }
        outFile << "\n";
        outFile.close();
    }

    void addPenalty(std::size_t id, double penalty) {
        std::find_if(cities.begin(), cities.end(), [id](const auto& val) { return val->getId() == id; })->get()->addPenalty(penalty);
    }

    std::size_t getCitiesAmount() const {
        return this->citiesAmount;
    }

    void makeGreedy() {
        for (std::size_t i{ 0 }; i < cities.size() - 1; ++i) {
            long float minWay = cities[i]->getWay(*cities[i+1]);
            std::size_t minJ = i + 1;
            for (std::size_t j{ i + 2 }; j < cities.size(); ++j) {
                long float way = cities[i]->getWay(*cities[j]);
                if (way < minWay) {
                    minWay = way;
                    minJ = j;
                }
            }
            std::iter_swap(cities.begin() + i + 1, cities.begin() + minJ);
        }
    }
};

class SolutionFinder {
private:
    std::unique_ptr<Solution> bestSolution;
    std::unique_ptr<Solution> currentSolution;
    long float bestLength;
public:
    SolutionFinder(const Solution& sol) {
        this->bestSolution.reset(new Solution(sol));
        this->currentSolution.reset(new Solution(sol));
        this->bestLength = this->bestSolution->calculateLength();
    }

    void greedy() {
        this->currentSolution->makeGreedy();
        *this->bestSolution = *this->currentSolution;
        this->bestLength = this->bestSolution->calculateLength();
    }

    void localSearch(std::size_t iterations, std::size_t citiesAmount, std::string filename = "result.txt") {
        std::size_t saved1 = 1, saved2 = 1;
        bool nextIter = false;
        for (std::size_t i{ 0 }; i < iterations; ++i) {
            //long float maxWin = 0;
            //std::size_t bestRep1 = 0, bestRep2 = 0;
            for (std::size_t id1 = saved1; id1 <= citiesAmount; ++id1) {
                std::size_t bestRep1 = 0, bestRep2 = 0;
                long float maxWin = 0;
                for (std::size_t id2 = id1 + 1; id2 <= citiesAmount; id2 += /*iterations - */i + 1) {
                    currentSolution->replaceRange(id1, id2); // should replace 2 cities, not the range (distance will be the same)
                    if (this->bestLength - currentSolution->calculateLength() > maxWin) {
                        bestRep1 = id1;
                        bestRep2 = id2;
                        //this->bestLength = this->currentSolution->calculateLength();
                        maxWin = this->bestLength - currentSolution->calculateLength();
                        //break;
                    }
                    //else
                        currentSolution->replaceRange(id1, id2);
                }
              
                if (bestRep1 != 0) {
                    currentSolution->replaceRange(bestRep1, bestRep2);
                    *this->bestSolution = *this->currentSolution;
                    this->bestLength = this->bestSolution->calculateLength();
                    //break;
                    bestRep1 = 0; bestRep2 = 0;
                }
                if (id1 % 10 == 0)
                {
                    system("cls");
                    std::cout << "[ " << static_cast<int>( static_cast<double>(id1) / citiesAmount * 100 ) << "% ] of #" << i << " iteration";

                    printBestLen();
                }
            }
            printBestWay(filename);
            //printBestLen();
        }
    }

    void printBestLen() const {
        std::cout << "\n\nLen: " << this->bestLength << "\n\n";
    }

    void printBestWay() const {
        this->bestSolution->print();
    }

    void printBestWay(std::string filename) const {
        this->bestSolution->print(filename);
    }
};

int main() {

    std::string fileInp{ "mona_1000.txt" };
    std::string fileRes{ "mona_1000_res.txt" };

    Solution tst(fileInp);

    SolutionFinder testFinder(tst);
    testFinder.greedy();
    std::cout << "Greedy: ";
    testFinder.printBestLen();
    std::cout << "\n\n";
    for(std::size_t i {0}; i < 5; ++i)
        testFinder.localSearch(20, tst.getCitiesAmount(), fileRes);

    //testFinder.printBestWay("mona_1000_res.txt");

    //std::vector<int> cities{ 1, 2, 3, 4, 5, 6, 7, 8 };
    //std::reverse(std::find_if(cities.begin(), cities.end(), [](const auto& val) { return val == 2; })
     //   , ++(std::find_if(cities.begin(), cities.end(), [](const auto& val) { return val == 6; })) );

//    std::copy(cities.begin(), cities.end(), std::ostream_iterator<int>(std::cout, " "));

    return 0;
}
