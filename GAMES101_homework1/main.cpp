#include "triangle.hpp"
#include "rasterizer.hpp"
#include <eigen-3.3.9/eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>


constexpr double my_pi = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0],
        0, 1, 0, -eye_pos[1],
        0, 0, 1, -eye_pos[2],
        0, 0, 0, 1;

    view = translate * view;

    return view;
}

/**
 * 逐个元素构建模型变换矩阵并返回该矩阵。
 * Details：实现三维中绕z轴旋转的变换矩阵，不用处理平移与缩放。
 * @param angle
 * @return
 */
Eigen::Matrix4f get_model_matrix(float angle)
{
    // todo: implement this function
    // create the model matrix for rotating the triangle around the z axis.
    // then return it.

    Eigen::Matrix4f rotation;
    angle = angle * my_pi / 180.0;

    rotation << cos(angle), 0, sin(angle), 0,
        0, 1, 0, 0,
        -sin(angle), 0, cos(angle), 0,
        0, 0, 0, 1;

    Eigen::Matrix4f scale;
    scale << 2.5, 0, 0, 0,
        0, 2.5, 0, 0,
        0, 0, 2.5, 0,
        0, 0, 0, 1;

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    return translate * rotation * scale;
}

/**
 * 使用给定的参数逐个元素地构建透视投影矩阵并返回该矩阵
 * @param eye_fov
 * @param aspect_ratio
 * @param znear
 * @param zfar
 * @return
 */
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float znear, float zfar)
{
    // students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f m_persp2ortho(4, 4);
    Eigen::Matrix4f m_ortho_scale(4, 4);
    Eigen::Matrix4f m_ortho_trans(4, 4);

    float angle = eye_fov * my_pi / 180.0;
    float height = znear * tan(angle) * 2;
    float width = height * aspect_ratio;

    auto t = -znear * tan(angle / 2);
    auto r = t * aspect_ratio;
    auto l = -r;
    auto b = -t;

    m_persp2ortho << znear, 0, 0, 0,
        0, znear, 0, 0,
        0, 0, znear + zfar, -znear * zfar,
        0, 0, 1, 0;

    m_ortho_scale << 2 / (r - l), 0, 0, 0,
        0, 2 / (t - b), 0, 0,
        0, 0, 2 / (znear - zfar), 0,
        0, 0, 0, 1;

    m_ortho_trans << 1, 0, 0, -(r + l) / 2,
        0, 1, 0, -(t + b) / 2,
        0, 0, 1, -(znear + zfar) / 2,
        0, 0, 0, 1;

    Eigen::Matrix4f m_ortho = m_ortho_scale * m_ortho_trans;
    projection = m_ortho * m_persp2ortho * projection;

    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}