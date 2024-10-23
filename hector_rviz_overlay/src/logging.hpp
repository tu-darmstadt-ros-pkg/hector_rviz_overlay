//
// Created by stefan on 07.06.24.
//

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <rclcpp/logging.hpp>

#define LOG_INFO( ... ) RCLCPP_INFO( rclcpp::get_logger( "hector_rviz_overlay" ), __VA_ARGS__ )
#define LOG_WARN( ... ) RCLCPP_WARN( rclcpp::get_logger( "hector_rviz_overlay" ), __VA_ARGS__ )
#define LOG_ERROR( ... ) RCLCPP_ERROR( rclcpp::get_logger( "hector_rviz_overlay" ), __VA_ARGS__ )

#endif // LOGGING_HPP
