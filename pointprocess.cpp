#include "pointprocess.h"

PointProcess::PointProcess(QObject *parent)
{
    Q_UNUSED(parent);
    QObject::connect(this, &PointProcess::signalCluster, this, &PointProcess::cluster);
}

PointProcess::~PointProcess()
{

}

vector<Point2f> PointProcess::toVectorPoint2f(vector<Point> _vec_point)
{
    vector<Point2f> _vec_point2f;
    if(_vec_point.empty()) {
        M_DEBUG("vector point input empty");
        return _vec_point2f;
    }
    for(size_t i = 0; i < _vec_point.size(); i++) {
        _vec_point2f.push_back((Point2f)_vec_point.at(i));
    }
    return _vec_point2f;
}

// to caculate center of group point
Point2f PointProcess::meansVectorPoints(vector<Point2f> _vec_point)
{
    if(_vec_point.empty()) {
        M_DEBUG("vector<Point> empty");
        return Point2f();
    }
    Point2f temp(0, 0);
    for(size_t i = 0; i < _vec_point.size(); i ++) {
        temp += _vec_point.at(i);
    }
    return Point2f(temp.x / (double)_vec_point.size(), temp.y / (double)_vec_point.size());
}

// get center and radius of group point
Point2f PointProcess::meansVectorPoints(vector<Point2f> _vec_point, double &radius)
{
    if(_vec_point.empty()) {
        M_DEBUG("vector<Point> empty");
        return Point2f();
    }
    Point2f temp(0, 0);
    double _vec_point_size = (double)_vec_point.size();
    for(size_t i = 0; i < _vec_point.size(); i++) {
        temp += _vec_point.at(i);
    }
    Point2f means(temp.x/_vec_point_size, temp.y/_vec_point_size);

    double _sum = 0;
    for(size_t i = 0; i < _vec_point.size(); i++) {
        _sum += norm(means - _vec_point.at(i));
    }
    radius = _sum/_vec_point_size;
    return means;
}

void PointProcess::hierarchicalClustering(vector<Point2f> point_list, double max_distance,
                                          int max_group, vector<vector<Point2f>> &group_point) {
    if(point_list.empty()) {
        M_DEBUG("error empty input");
        return;
    }
    if(max_distance <= 0) {
        M_DEBUG("error distance input");
        return;
    }
    vector<bool> check(point_list.size(), false);
    vector<Point2f> temp;
    for(size_t i = 0; i < point_list.size(); i++) {
        if(check.at(i)) {
            continue;
        } else {
            temp.push_back(point_list.at(i));
            check.at(i) = true;
            for( size_t j = i + 1; j < point_list.size(); j++) {
                if(check.at(j)) {
                    continue;
                } else {
                    if( norm(point_list.at(i) - point_list.at(j)) <= max_distance) {
                        temp.push_back(point_list.at(j));
                        check.at(j) = true;
                    }
                }
            }
            group_point.push_back(temp);
            temp.clear();
            if(group_point.size() >= (size_t)max_group) {
                break;
            }
        }
    }
    Debug::_delete(check, temp);
}

void PointProcess::hierarchicalClustering(vector<Point2f> point_list, double max_distance,
                                          vector<Point2f> ref_list, int max_group,
                                          vector<vector<Point2f>> &group_point)
{
    if(point_list.empty()) {
        M_DEBUG("error empty input");
        return;
    }
    if(max_distance <= 0) {
        M_DEBUG("error distance input");
        return;
    }
    vector<bool> check(point_list.size(), false);
    vector<Point2f> temp;

    for(size_t i = 0; i < ref_list.size(); i++) {
        for( size_t j = 0; j < point_list.size(); j++) {
            if(check.at(j)) {
                continue;
            } else {
                if( norm(point_list.at(j) - ref_list.at(i)) <= max_distance) {
                    temp.push_back(point_list.at(j));
                    check.at(j) = true;
                }
            }
        }
        group_point.push_back(temp);
        temp.clear();
        if(group_point.size() >= (size_t)max_group) {
            break;
        }
    }
    Debug::_delete(check, temp);
}

void PointProcess::hierarchicalClustering(vector<Point2f> point_list, double max_distance,
                                          int max_group, vector<vector<int>> &pointInGroup_idx) {
    if(point_list.empty()) {
        M_DEBUG("error empty input");
        return;
    }
    if(max_distance <= 0) {
        M_DEBUG("error distance input");
        return;
    }
    vector<bool> check(point_list.size(), false);
    vector<int> temp_idx;

    for(size_t i = 0; i < point_list.size(); i++) {
        if(check.at(i)) {
            continue;
        } else {
            temp_idx.push_back(i);
            check.at(i) = true;
            for( size_t j = i + 1; j < point_list.size(); j++) {
                if(check.at(j)) {
                    continue;
                } else {
                    if( norm(point_list.at(i) - point_list.at(j)) < max_distance) {
                        temp_idx.push_back(j);
                        check.at(j) = true;
                    }
                }
            }
            pointInGroup_idx.push_back(temp_idx);
            temp_idx.clear();
            if(pointInGroup_idx.size() >= (size_t)max_group) {
                break;
            }
        }
    }
    Debug::_delete(check, temp_idx);
}

void PointProcess::drawAxis(Mat& img, Point p, Point q, Scalar colour, const float scale)
{
    double angle = atan2( (double) p.y - q.y, (double) p.x - q.x ); // angle in radians
    double hypotenuse = sqrt( (double) (p.y - q.y) * (p.y - q.y) + (p.x - q.x) * (p.x - q.x));
    // Here we lengthen the arrow by a factor of scale
    q.x = (int) (p.x - scale * hypotenuse * cos(angle));
    q.y = (int) (p.y - scale * hypotenuse * sin(angle));
    line(img, p, q, colour, 1, LINE_AA);
    // create the arrow hooks
    p.x = (int) (q.x + 9 * cos(angle + CV_PI / 4));
    p.y = (int) (q.y + 9 * sin(angle + CV_PI / 4));
    line(img, p, q, colour, 1, LINE_AA);
    p.x = (int) (q.x + 9 * cos(angle - CV_PI / 4));
    p.y = (int) (q.y + 9 * sin(angle - CV_PI / 4));
    line(img, p, q, colour, 1, LINE_AA);
}

double PointProcess::getAngle(vector<Point2f> vec_contour, Mat &color_img)
{
    if(vec_contour.empty()) {
        M_DEBUG("vector contour is empty");
        return 0;
    }
    //Construct a buffer used by the pca analysis
    int vec_contour_size = static_cast<int>(vec_contour.size());
    Mat data_vec_contour = Mat(vec_contour_size, 2, CV_64F);
    for (int i = 0; i < data_vec_contour.rows; i++) {
        data_vec_contour.at<double>(i, 0) = vec_contour[i].x;
        data_vec_contour.at<double>(i, 1) = vec_contour[i].y;
    }

    //Perform PCA analysis
    PCA pca_analysis(data_vec_contour, Mat(), PCA::DATA_AS_ROW);
    //Store the center of the object
    Point cntr = Point(static_cast<int>(pca_analysis.mean.at<double>(0, 0)),
                       static_cast<int>(pca_analysis.mean.at<double>(0, 1)));

    //Store the eigenvalues and eigenvectors
    vector<Point2d> eigen_vecs(2);
    vector<double> eigen_val(2);
    for (int i = 0; i < 2; i++) {
        eigen_vecs[i] = Point2d(pca_analysis.eigenvectors.at<double>(i, 0),
                                pca_analysis.eigenvectors.at<double>(i, 1));
        eigen_val[i] = pca_analysis.eigenvalues.at<double>(i);
    }
    // Draw the principal components
    circle(color_img, cntr, 3, Scalar(255, 0, 255), 2);
    Point p1 = cntr + 0.02 * Point(static_cast<int>(eigen_vecs[0].x * eigen_val[0]),
                                   static_cast<int>(eigen_vecs[0].y * eigen_val[0]));
    Point p2 = cntr - 0.02 * Point(static_cast<int>(eigen_vecs[1].x * eigen_val[1]),
                                   static_cast<int>(eigen_vecs[1].y * eigen_val[1]));
    drawAxis(color_img, cntr, p1, Scalar(0, 255, 0), 1);
    drawAxis(color_img, cntr, p2, Scalar(255, 255, 0), 5);
    double angle = atan2(eigen_vecs[0].y, eigen_vecs[0].x)*180.0/CV_PI; // orientation in radians
    return angle;
}

void PointProcess::filledPara(vector<Point2f> contour, PointProcess::Object_t &object,
                              Mat &color_image)
{
    if(contour.empty()) {
        M_DEBUG("contour empty");
        return;
    }
    double radius;
    // caculate center, radius and angle of countour point.
    Point2f center = meansVectorPoints(contour, radius);
//    qDebug() <<"raw" << radius;
//    double angle = getAngle(contour, color_image);

    // assign to object
    object.center = center;
    object.radius_img = radius_filter.updateEstimate(radius);
//    object.angle = angle_filter.updateEstimate(angle);
}

void PointProcess::setVecContour(vector<vector<Point> > _vec_contour, Mat& color_image)
{
    ready_get_flag = false;
    // pop front
    vector<Object_t>::iterator last_idx = objects_raw.begin();
    advance(last_idx, object_per_frame[0]);
    objects_raw.erase(objects_raw.begin(), last_idx);

    // push back
    Object_t object_temp;
    vector<Point2f> contour_2f;
    foreach (vector<Point> contour, _vec_contour) {
        contour_2f = toVectorPoint2f(contour);
        filledPara(contour_2f, object_temp, color_image);
        objects_raw.push_back(object_temp);
//        qDebug() <<"filter" << object_temp.radius_img;
    }
    // set index
    int num_contour = (int)_vec_contour.size();
    for(int i = 0; i < MAX_NUM_SIZE - 1; i++) {
        object_per_frame[i] = object_per_frame[i+1];
    }
    object_per_frame[MAX_NUM_SIZE - 1] = num_contour;

    Debug::_delete(contour_2f);
    emit signalCluster();

}

void PointProcess::cluster()
{
    vector<vector<int>> groups_idx;
    vector<Point2f> list_center;
    foreach (Object_t object, objects_raw) {
        list_center.push_back(object.center);
    }
    hierarchicalClustering(list_center, ACURACY_GROUP_SIZE, MAX_GROUP_NUM, groups_idx);

    objects_cluster.clear();
    foreach (vector<int> group_idx, groups_idx) {
        Object_t object_temp = {Point2f(0, 0), 0, 0};
        foreach (int idx, group_idx) {
            object_temp.angle += objects_raw.at(idx).angle;
            object_temp.radius_img += objects_raw.at(idx).radius_img;
            object_temp.center += objects_raw.at(idx).center;
        }
        object_temp.angle /= (double)group_idx.size();
        object_temp.radius_img /= (double)group_idx.size();
        object_temp.center = Point2f(object_temp.center.x/(double)group_idx.size(),
                                     object_temp.center.y/(double)group_idx.size());
//        qDebug() <<"cluster" << object_temp.radius_img;
        objects_cluster.push_back(object_temp);
    }

    ready_get_flag = true;
    Debug::_delete(groups_idx, list_center);

}

bool PointProcess::isReadyGet(vector<Object_t> &_vec_object)
{
    if(ready_get_flag) {
        if(!_vec_object.empty()) {
            _vec_object.clear();
        }
        _vec_object.insert(_vec_object.begin(), objects_cluster.begin(), objects_cluster.end());
    }
    return ready_get_flag;
}
