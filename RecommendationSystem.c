#include "RecommendationSystem.h"
#include <cmath>
#include <algorithm>

RecommendationSystem::RecommendationSystem() {}

double RecommendationSystem::get_average(const User& user){
  double average_rating = 0.0;
  for (const auto& pair : user.get_ranks())
    {
      const double rating = pair.second;
      average_rating += rating;
    }
  int movies_num = user.get_ranks().size();
  average_rating /= movies_num;
  return average_rating;
}



std::vector<double> RecommendationSystem::add_vectors
(const std::vector<double>& vec1, const std::vector<double>& vec2){
  std::vector<double> result = vec1;
  for (size_t i = 0; i < result.size(); ++i)
    {
      result[i] += vec2[i];
    }
  return result;
}


std::vector<double>  RecommendationSystem::multiply_vectors
(const std::vector<double>& vec1,const std::vector<double>& vec2)
{
  std::vector<double> result;
  if (vec1.size() != vec2.size()) {
      return result;
    }
  result.reserve(vec1.size());
  for (size_t i = 0; i < vec1.size(); ++i) {
      result.push_back(vec1[i] * vec2[i]);
    }
  return result;
}

double RecommendationSystem::sum_vector
(const std::vector<double>& vec1, const std::vector<double>& vec2) {
  double total = 0.0;
  std::vector<double> result;
  result= multiply_vectors (vec1,vec2);
  // Iterate over each element in the vector and add it to the total
  for (double value : result) {
      total += value;
    }
  return total;
}

double RecommendationSystem::norm_vector (const std::vector<double> &vector)
{
  double norm = 0.0;
  for (double value : vector) {
      norm += value * value;
    }
  norm = std::sqrt(norm);
  return norm;
}



std::vector<double> RecommendationSystem::get_features
(const sp_movie& movie,const ft_map &features)
{
  auto it = features.find (movie);
  if (it != features.end ())
    {
      return it->second;
    }
  return {};
}


sp_movie RecommendationSystem::recommend_by_content(const User& user){
  double average_rating = get_average (user);
  std::vector<double>  user_preferences;
  for (const auto& pair : user.get_ranks()){
  const std::vector<double>& movie_features = get_features(pair.first,_ft_map);
  double normalized_rating =  pair.second - average_rating;
  std::vector<double> user_preference(movie_features.size(), 0.0);
  for (size_t i = 0; i < movie_features.size(); ++i){
       user_preference[i] += normalized_rating * movie_features[i];
   }
      user_preferences.resize (movie_features.size());
      user_preferences = add_vectors(user_preference,user_preferences);
  }
  sp_movie recommended_movie;
  double max_similarity = -1.0;
  double norm_user_preferences_vector = norm_vector(user_preferences);
  for(auto item :_ft_map){
      const sp_movie& movie = item.first;
      const rank_map& user_rankings = user.get_ranks();
      if(user_rankings.find(movie) != user_rankings.end()){
        continue;}
      double similarity = get_similiraty
          (user_preferences, norm_user_preferences_vector, item);
      if (similarity > max_similarity){
          recommended_movie = item.first;
          max_similarity = similarity;
      }
  }
  return recommended_movie;
}
double RecommendationSystem::get_similiraty
(const std::vector<double> &user_preferences,
 double norm_user_preferences_vector,
 const std::pair<const std::shared_ptr<Movie>
     , std::vector<double>> &item)
{
  const std::vector<double>& movie_features = get_features
      (item.first, _ft_map);
  double res1 = sum_vector (user_preferences, movie_features);
  double norm_movie_features = norm_vector (movie_features);
  double res2 = norm_movie_features * norm_user_preferences_vector;
  double similarity = res1 / res2;
  return similarity;
}

sp_movie RecommendationSystem::add_movie
(const std::string &name, int year, const std::vector<double> &features) {
  sp_movie movie = std::make_shared<Movie>(name, year);
  _ft_map[movie] = features;
  return movie;
}


sp_movie RecommendationSystem::get_movie
(const std::string &name, int year) const{
  for (const auto &movie: _ft_map)
    {
      if (movie.first->get_name () == name && movie.first->get_year () == year)
        {
          return movie.first;
        }
    }
  return nullptr;
}



std::vector<sp_movie> RecommendationSystem::get_sorted_movies
(const RecommendationSystem &recsys) {
  std::vector<sp_movie> sorted_movies;
  for (const auto& pair : recsys._ft_map) {
      sorted_movies.push_back(pair.first);
    }
    std::sort(sorted_movies.begin(), sorted_movies.end(),
   [](const sp_movie& movie1, const sp_movie& movie2) {
                return *movie1 < *movie2;
            });
  return sorted_movies;
}


std::ostream &operator<<
(std::ostream &os, const RecommendationSystem &recsys){
  std::vector<sp_movie> sorted_movies = recsys.get_sorted_movies(recsys);
  for (const auto& movie : sorted_movies) {
      os << *movie << std::endl;
    }

  return os;
}


sp_movie RecommendationSystem::recommend_by_cf (const User &user, int k)
{
  rank_map user_rankings = user.get_ranks();
  sp_movie recommended_movie;
  double highest_score = -1.0;
  for (const auto& pair : _ft_map) {
      sp_movie movie = pair.first;
      if (user_rankings.find(movie) != user_rankings.end())
        { continue; }
        double score = predict_movie_score(user, movie, k);
      if (score > highest_score) {
          recommended_movie = movie;
          highest_score = score;
        }
    }
  return recommended_movie;
}


double RecommendationSystem::calculate_similarity
(const sp_movie &movie1, const sp_movie &movie2){
  std::vector<double> movie1_features = get_features(movie1, _ft_map);
  std::vector<double> movie2_features = get_features(movie2, _ft_map);
  double res1 = sum_vector (movie1_features, movie2_features);
  double norm_vector_movie1 = norm_vector (movie1_features);
  double norm_vector_movie2 = norm_vector (movie2_features);
  double res2 = norm_vector_movie1 * norm_vector_movie2;
  double similarity = res1 / res2;
  return similarity;
}

std::vector<std::pair<sp_movie, double>>
RecommendationSystem::find_k_similar_movies
(const User &user, const sp_movie &movie1, int k){
  std::vector<std::pair<sp_movie,double>> similar_movies;
  for (const auto& pair : user.get_ranks())
    {
      const sp_movie& movie2 = pair.first;
      double similarity = calculate_similarity(movie1, movie2);

      similar_movies.emplace_back(movie2, similarity);
    }
  std::sort(similar_movies.begin(),similar_movies.end(),
 [&](const std::pair<sp_movie, double>& m1,
     const std::pair<sp_movie, double>& m2) {
      return m1.second > m2.second;
  });
  if ((int)(similar_movies.size()) > k) {
      similar_movies.resize(k);
    }
  return similar_movies;
}

double RecommendationSystem::predict_movie_score
(const User &user, const sp_movie &movie, int k){
  std::vector<std::pair<sp_movie, double>>
  similar_movies = find_k_similar_movies(user, movie, k);
  double weighted_sum = 0.0;
  double similarity_sum = 0.0;
  for (const auto& mov : similar_movies) {
      sp_movie similar_movie = mov.first;
      double similarity = mov.second;
      auto map = user.get_ranks();
      auto rank_it = map.find(similar_movie);
      if (rank_it != user.get_ranks().end()) {
          double rating = rank_it->second;
          weighted_sum += rating * similarity;
          similarity_sum += similarity;
        }
    }
  double predicted_score = (similarity_sum > 0.0)
      ? (weighted_sum / similarity_sum) : 0.0;
  return predicted_score;
}
