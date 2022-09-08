function ret = data_received(element)
    obj.data(end + 1) = element;
    ret = size(obj.data);
end