X = load('../mat_train_hists.csv');     
    
% Set parameters
no_dims = 2;
initial_dims = 16;
perplexity = 20;

% Run t−SNE
mappedX = tsne(X, [], no_dims, initial_dims, perplexity);
% Plot results
gscatter(mappedX(:,1), mappedX(:,2), []);