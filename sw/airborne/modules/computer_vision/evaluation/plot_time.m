function [] = plot_time( coords)
%PLOT_TIME Summary of this function goes here
%   Detailed explanation goes here

figure;
for i = 1:length(coords)
   
    scatter(coords(1:i, 1), coords(1:i, 2), 'MarkerFaceColor',[1 .0 .0], 'MarkerEdgeColor',[1 .0 .0])
    xlim([0 max(1280)])
    ylim([0 max(800)])
    drawnow
%     hold on;
%     scatter(coords2(1:i, 1), coords2(1:i, 2), 'MarkerFaceColor',[0 1. .0], 'MarkerEdgeColor',[0 1. .0])
    
    pause(0.0000001);
    
end

end
