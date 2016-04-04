clear, clc;

textons = load('../textons.csv');


denom = (max(textons, [], 2) - min(textons, [], 2));
t_out = zeros(size(textons));


for i = 1:33
   if ((max(textons(i, :)) - min(textons(i, :))) > 0.01)
       t_out(i, :) =  (textons(i, :) - min(textons(i, :))) / (max(textons(i, :)) - min(textons(i, :)));
   end
end
% t_out = (textons - min(textons, [], 2)) ./ denom;


textons_res = reshape(t_out', [5 5 33])

N = 33;
colormap gray;
for i = 1:N
    subplot(6, 6, i)
    imshow(textons_res(:, :, i))
    
end

% dlmwrite('textons_std.csv', textons, ...
% 'delimiter',',','precision','%0.4f')

% csvwrite('textons_std.csv', t_out)