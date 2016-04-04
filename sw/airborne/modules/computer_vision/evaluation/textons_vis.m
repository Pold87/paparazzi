clear, clc;
X = load('../textons.csv')

% textons_res = reshape(X, [5 5 33])
% 
% N = 33;
% colormap gray;
% for i = 1:N
%     subplot(6, 6, i)
%     imshow(textons_res(:, :, i))
%     
% end

A = prdataset(X) ;
A_pca = A * pcam(A,2);


scatter(A_pca(:, 1), A_pca(:, 2))

b = num2str((1:33)'); c = cellstr(b);
dx = 0.1; dy = 0; % displacement so the text does not overlay the data points
text(+A_pca(:, 1)+dx, +A_pca(:, 2)+dy, c);

% % Set parameters
% no_dims = 2;
% initial_dims = 15;
% perplexity = 8;
    
% Run t−SNE
% mappedX = tsne(X, [], no_dims, initial_dims, perplexity);
% Plot results
% gscatter(mappedX(:,1), mappedX(:,2), []);