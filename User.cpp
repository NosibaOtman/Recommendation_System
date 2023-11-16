

// don't change those includes
#include "User.h"
#include "RecommendationSystem.h"


// implement your cpp code here
#include <utility>


User::User(std::string name, rank_map ranks,
           const std::shared_ptr<RecommendationSystem>& resys)
    : _username(std::move(name)), _ranks(std::move(ranks)), _resys(resys) {}

std::string User::get_name() const {
  return _username;
}

void User::add_movie_to_rs(const std::string& name, int year,
       const std::vector<double>& features, double rate) {
  sp_movie movie = _resys->add_movie(name, year, features);
  _ranks[movie] = rate;
}

rank_map User::get_ranks() const {
  return _ranks;
}

sp_movie User::get_recommendation_by_content() const {
  return _resys->recommend_by_content(*this);
}

sp_movie User::get_recommendation_by_cf(int k) const {
  return _resys->recommend_by_cf(*this, k);
}

double User::get_prediction_score_for_movie
(const std::string& name, int year, int k) const {
  sp_movie movie = _resys->get_movie(name, year);
  if (movie)
    { return _resys->predict_movie_score (*this, movie, k); }
  else
    { return 0.0; }
}

std::ostream& operator<<(std::ostream& os, const User& user) {
  os << "User: " << user._username << std::endl;
  for (const auto& pair : user._ranks) {
      os << pair.first->get_name()
      << " (" << pair.first->get_year() << "): " << pair.second << std::endl;
    }
  return os;
}
