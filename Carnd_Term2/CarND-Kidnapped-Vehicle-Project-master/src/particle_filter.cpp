#include "particle_filter.h"

#include <math.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <time.h>

#include "helper_functions.h"

using namespace std;

const int NUMBER_OF_PARTICLES = 600;
const double EPS = 0.001;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
  /**
   * TODO: Set the number of particles. Initialize all particles to 
   *   first position (based on estimates of x, y, theta and their uncertainties
   *   from GPS) and all weights to 1. 
   * TODO: Add random Gaussian noise to each particle.
   * NOTE: Consult particle_filter.h for more information about this method 
   *   (and others in this file).
   */
    num_particles = 0;  // TODO: Set the number of particles
	static default_random_engine gen;
	gen.seed(time(NULL));
	num_particles = NUMBER_OF_PARTICLES;
	normal_distribution<double> x_dist(x, std[0]);
	normal_distribution<double> y_dist(y, std[1]);
	normal_distribution<double> theta_dist(theta, std[2]);
	particles.resize(num_particles);
	weights.resize(num_particles);
	double init_weight =  1.0 / num_particles;

	for(int i = 0; i < num_particles; i++){
	    particles[i].id = i;
	    particles[i].x = x_dist(gen);
	    particles[i].y = y_dist(gen);
	    particles[i].theta = theta_dist(gen);
	    particles[i].weight = init_weight;
	}
	is_initialized = true;
	
}

void ParticleFilter::prediction(double delta_t, double std_pos[], 
                                double velocity, double yaw_rate) {
  /**
   * TODO: Add measurements to each particle and add random Gaussian noise.
   * NOTE: When adding noise you may find std::normal_distribution 
   *   and std::default_random_engine useful.
   *  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
   *  http://www.cplusplus.com/reference/random/default_random_engine/
   */
    const double vel_d_t = velocity * delta_t;
    const double yaw_d_t = yaw_rate * delta_t;
    const double vel_yaw = velocity/yaw_rate;
    static default_random_engine gen;
    gen.seed(time(NULL));
    normal_distribution<double> x_dist(0.0, std_pos[0]);
    normal_distribution<double> y_dist(0.0, std_pos[1]);
    normal_distribution<double> theta_dist(0.0, std_pos[2]);
    for (int i = 0; i < num_particles; i++){
        if (fabs(yaw_rate) < EPS){
            particles[i].x += vel_d_t * cos(particles[i].theta);
            particles[i].y += vel_d_t * sin(particles[i].theta);
            // particles[i].theta unchanged if yaw_rate is too small
        }
        else{
            const double theta_new = particles[i].theta + yaw_d_t;
            particles[i].x += vel_yaw * (sin(theta_new) - sin(particles[i].theta));
            particles[i].y += vel_yaw * (-cos(theta_new) + cos(particles[i].theta));
            particles[i].theta = theta_new;
        }
        // Add random Gaussian noise
        particles[i].x += x_dist(gen);
        particles[i].y += y_dist(gen);
        particles[i].theta += theta_dist(gen);
    }

}

void ParticleFilter::dataAssociation(vector<LandmarkObs> predicted, 
                                     vector<LandmarkObs>& observations) {
  /**
   * TODO: Find the predicted measurement that is closest to each 
   *   observed measurement and assign the observed measurement to this 
   *   particular landmark.
   * NOTE: this method will NOT be called by the grading code. But you will 
   *   probably find it useful to implement this method and use it as a helper 
   *   during the updateWeights phase.
   */

}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
                                   const vector<LandmarkObs> &observations, 
                                   const Map &map_landmarks) {
    /**
     * TODO: Update the weights of each particle using a mult-variate Gaussian
     *   distribution. You can read more about this distribution here:
     *   https://en.wikipedia.org/wiki/Multivariate_normal_distribution
     * NOTE: The observations are given in the VEHICLE'S coordinate system.
     *   Your particles are located according to the MAP'S coordinate system.
     *   You will need to transform between the two systems. Keep in mind that
     *   this transformation requires both rotation AND translation (but no scaling).
     *   The following is a good resource for the theory:
     *   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
     *   and the following is a good resource for the actual equation to implement
     *   (look at equation 3.33) http://planning.cs.uiuc.edu/node99.html
     */
    const double sigma_xx = std_landmark[0] * std_landmark[0];
    const double sigma_yy = std_landmark[1] * std_landmark[1];
    const double k = 2 * M_PI * std_landmark[0] * std_landmark[1];
    double dx = 0.0;
    double dy = 0.0;
    double sum_w = 0.0; // Sum of weights for future weights normalization
    for (int i = 0; i < num_particles; i++){
        double weight_no_exp = 0.0;
        const double sin_theta = sin(particles[i].theta);
        const double cos_theta = cos(particles[i].theta);
        for (int j = 0; j < observations.size(); j++){
            // Observation measurement transformations
            LandmarkObs observation;
            observation.id = observations[j].id;
            observation.x = particles[i].x + (observations[j].x * cos_theta) - (observations[j].y * sin_theta);
            observation.y = particles[i].y + (observations[j].x * sin_theta) + (observations[j].y * cos_theta);
            // Unefficient way for observation asossiation to landmarks. It can be improved.
            bool in_range = false;
            Map::single_landmark_s nearest_lm;
            double nearest_dist = 10000000.0; // A big number
            for (int k = 0; k < map_landmarks.landmark_list.size(); k++) {
                Map::single_landmark_s cond_lm = map_landmarks.landmark_list[k];
                double distance = dist(cond_lm.x_f, cond_lm.y_f, observation.x, observation.y);  // Calculate the Euclidean distance between two 2D points
                if (distance < nearest_dist) {
                    nearest_dist = distance;
                    nearest_lm = cond_lm;
                    if (distance < sensor_range){
                        in_range = true;
                    }
                }
            }
            if (in_range){
                dx = observation.x-nearest_lm.x_f;
                dy = observation.y-nearest_lm.y_f;
                weight_no_exp += dx * dx / sigma_xx + dy * dy / sigma_yy;
            }
            else {
                weight_no_exp += 100; // approx = 0 after exp()
            }
        }
        particles[i].weight = exp(-0.5*weight_no_exp); // calculate exp() after main computation in order to optimize the code
        sum_w += particles[i].weight;
    }
    // Weights normalization to sum(weights)=1
    for (int i = 0; i < num_particles; i++){
        particles[i].weight /= sum_w * k;
        weights[i] = particles[i].weight;
    }
}
void ParticleFilter::resample() {
  /**
   * TODO: Resample particles with replacement with probability proportional 
   *   to their weight. 
   * NOTE: You may find std::discrete_distribution helpful here.
   *   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
   */
    static default_random_engine gen;
    gen.seed(time(NULL));
    discrete_distribution<> dist_particles(weights.begin(), weights.end());
    vector<Particle> new_particles;
    new_particles.resize(num_particles);
    for (int i = 0; i < num_particles; i++) {
        new_particles[i] = particles[dist_particles(gen)];
    }
    particles = new_particles;

}

void ParticleFilter::SetAssociations(Particle& particle, 
                                     const vector<int>& associations, 
                                     const vector<double>& sense_x, 
                                     const vector<double>& sense_y) {
  // particle: the particle to which assign each listed association, 
  //   and association's (x,y) world coordinates mapping
  // associations: The landmark id that goes along with each listed association
  // sense_x: the associations x mapping already converted to world coordinates
  // sense_y: the associations y mapping already converted to world coordinates
  particle.associations= associations;
  particle.sense_x = sense_x;
  particle.sense_y = sense_y;
}

string ParticleFilter::getAssociations(Particle best) {
  vector<int> v = best.associations;
  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<int>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length()-1);  // get rid of the trailing space
  return s;
}

string ParticleFilter::getSenseCoord(Particle best, string coord) {
  vector<double> v;

  if (coord == "X") {
    v = best.sense_x;
  } else {
    v = best.sense_y;
  }

  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<float>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length()-1);  // get rid of the trailing space
  return s;
}